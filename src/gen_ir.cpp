#include "gen_ir.h"

IRModule& IRGenerator::run(){

    module.funcPool.clear();
    // Bind the translation unit
    bindTU(root);

    // generation of main function
    ASTNode rootNode = ps.getAST(root);
    for(auto astIdx : rootNode.body){
        ASTNode& astNode = ps.getAST(astIdx);
        if(astNode.kind == ASTKind::Import){
            // Ignore imports during code generation
        } else if(astNode.kind == ASTKind::Function){
            module.funcPool.push_back(genFunc(astIdx));
        } else {
            fprintf(stderr, "Unexpected AST node in TranslationUnit during IR generation\n");
            exit(1);
        }
    }

    return module;
}

void IRGenerator::bindTU(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    for(auto topIdx : node.body){
        ASTNode topNode = ps.getAST(topIdx);
        if(topNode.kind == ASTKind::Import){
            bindImport(topIdx);
        } else if(topNode.kind == ASTKind::Function){
            bindFunc(topIdx);
        } else {
            fprintf(stderr, "Unexpected AST node in TranslationUnit\n");
            exit(1);
        }
    }
}

void IRGenerator::bindImport(ASTIdx idx){
    ASTNode node = ps.getAST(idx);
    std::string moduleName = node.name;

    auto symbols = tnlibLoader.loadTnlib(moduleName);
    for(auto& sym : symbols){
        module.insertSymbol(sym);
    }
}

void IRGenerator::bindFunc(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    Symbol sym;
    sym.name = node.name;
    sym.setPub(node.isPub());
    sym.setMut(false); // functions are not mutable
    sym.kind = SymbolKind::Function;

    module.currentStackSize = 0;
    module.scopeIn();

    // Add function parameters to symbol table
    std::vector<SymbolIdx> paramSyms;
    for(auto paramIdx : node.params){
        ASTNode paramNode = ps.getAST(paramIdx);
        Symbol paramSym;
        paramSym.name = paramNode.name;
        paramSym.setMut(true);
        paramSym.kind = SymbolKind::Variable;
        SymbolIdx symidx = module.insertSymbol(paramSym);
        paramSyms.push_back(symidx);
    }
    FuncSem& fs = module.funcSem[idx];
    fs.params = paramSyms;

    bindStmt(node.body[0]);  // function body
    module.scopeOut();

    sym.params = paramSyms;
    module.insertSymbol(sym);

    module.funcSem[idx].localBytes = module.currentStackSize;
}

void IRGenerator::bindStmt(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::Return: {
            bindExpr(node.lhs);
            break;
        }
        case ASTKind::CompoundStmt: {
            module.scopeIn();
            for(auto stmtIdx : node.body){
                bindStmt(stmtIdx);
            }
            module.scopeOut();
            break;
        }
        case ASTKind::If: {
            bindExpr(node.cond);
            bindStmt(node.thenBr);
            if(node.elseBr != -1){
                bindStmt(node.elseBr);
            }
            break;            
        }
        case ASTKind::While: {
            bindExpr(node.cond);

            // currently body must have only one compound statement
            bindStmt(node.body[0]);
            break;            
        }
        case ASTKind::VarDecl: {
            Symbol sym;
            sym.name = node.name;
            sym.setMut(node.isMut());

            module.insertSymbol(sym);
            break;
        }
        case ASTKind::Assign: {
            bindExpr(node.lhs);
            bindExpr(node.rhs);
            break;
        }
        default:
            bindExpr(idx);
            break;
    }
}

void IRGenerator::bindExpr(ASTIdx idx){
    ASTNode& node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::Num:
            // nothing to do
            return;
        case ASTKind::StringLiteral:{
            // nothing to do
            int32_t strIdx = module.addString(node.str);
            node.val = strIdx;
            return;
        }
        case ASTKind::Variable: {
            SymbolIdx symIdx = module.findSymbol(node.name, module.curScope);
            if(symIdx == -1){
                fprintf(stderr, "Undefined variable: %s\n", node.name.c_str());
                exit(1);
            }
            module.astSymMap[idx] = symIdx;
            return;
        }
        case ASTKind::FunctionCall:{
            SymbolIdx symIdx = module.findSymbol(node.name, module.curScope);
            if(symIdx == -1){
                fprintf(stderr, "Undefined function: %s\n", node.name.c_str());
                exit(1);
            }
            module.astSymMap[idx] = symIdx;
            for(auto argIdx : node.args){
                bindExpr(argIdx);
            }
            return;
        }
        case ASTKind::Switch: {
            bindExpr(node.cond);
            for(auto caseIdx : node.body){
                ASTNode caseNode = ps.getAST(caseIdx);
                bindExpr(caseNode.lhs);

                module.scopeIn();
                bindExpr(caseNode.rhs);
                module.scopeOut();
            }
            return;
        }
        default:
            break;
    }

    if(node.lhs != -1)
        bindExpr(node.lhs);
    if(node.rhs != -1)
        bindExpr(node.rhs);
}

IRFunc IRGenerator::genFunc(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    func.clean();
    func.fname = node.name;

    FuncSem fs = module.funcSem[idx];
    func.localStackSize = fs.localBytes;
    func.params.clear();
    func.params = fs.params;        

    genStmt(node.body[0]);  // function body

    return func;
}

void IRGenerator::genStmt(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::Return: {
            IRInstr instr;
            instr.cmd = IRCmd::RET;
            instr.s1 = genExpr(node.lhs);
            func.instrPool.push_back(instr);
            break;
        }
        case ASTKind::CompoundStmt: {
            for(auto stmtIdx : node.body){
                genStmt(stmtIdx);
            }
            break;
        }
        case ASTKind::If: {
            VRegID condVid = genExpr(node.cond);

            // jump to else label if cond is zero
            IRInstr jz;
            jz.cmd = IRCmd::JZ;
            jz.s1 = condVid;
            jz.imm = func.newLabel();
            func.instrPool.push_back(jz);

            // then branch
            genStmt(node.thenBr);

            // if there is an else branch, jump to end label
            if(node.elseBr != -1){
                IRInstr jmpEnd;
                jmpEnd.cmd = IRCmd::JMP;
                jmpEnd.imm = func.newLabel();
                func.instrPool.push_back(jmpEnd);

                // else label
                IRInstr elseLabel;
                elseLabel.cmd = IRCmd::LLABEL;
                elseLabel.imm = jz.imm;
                func.instrPool.push_back(elseLabel);

                // else branch
                genStmt(node.elseBr);

                // end label
                IRInstr endLabel;
                endLabel.cmd = IRCmd::LLABEL;
                endLabel.imm = jmpEnd.imm;
                func.instrPool.push_back(endLabel);
            } else {
                // else label
                IRInstr elseLabel;
                elseLabel.cmd = IRCmd::LLABEL;
                elseLabel.imm = jz.imm;
                func.instrPool.push_back(elseLabel);
            }
            break;            
        }
        case ASTKind::While: {
            int32_t while_slabel = func.newLabel();
            int32_t while_elabel = func.newLabel();

            // start label
            IRInstr startLabel;
            startLabel.cmd = IRCmd::LLABEL;
            startLabel.imm = while_slabel;
            func.instrPool.push_back(startLabel);

            // condition
            VRegID condVid = genExpr(node.cond);

            // jump to end label if cond is zero
            IRInstr jz;
            jz.cmd = IRCmd::JZ;
            jz.s1 = condVid;
            jz.imm = while_elabel;
            func.instrPool.push_back(jz);

            // currently body must have only one compound statement
            genStmt(node.body[0]);

            // jump back to start
            IRInstr jmpStart;
            jmpStart.cmd = IRCmd::JMP;
            jmpStart.imm = while_slabel;
            func.instrPool.push_back(jmpStart);

            // end label
            IRInstr endLabel;
            endLabel.cmd = IRCmd::LLABEL;
            endLabel.imm = while_elabel;
            func.instrPool.push_back(endLabel);
            break;            
        }
        case ASTKind::VarDecl: {
            // Currently, do nothing for variable declarations
            break;
        }
        case ASTKind::Assign: {
            VRegID rhsVid = genExpr(node.rhs);

            VRegID lhsAddrVid = genlvalue(node.lhs);

            IRInstr instr;
            instr.cmd = IRCmd::SAVE;
            instr.s1 = lhsAddrVid;
            instr.s2 = rhsVid;
            func.instrPool.push_back(instr);
            break;        
        }
        default:
            genExpr(idx);
            break;
    }
}

VRegID IRGenerator::genExpr(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::Num:
            {
                return func.newVRegNum(node.val);
            }
        case ASTKind::StringLiteral:{
            VRegID vid = func.newVReg();
            IRInstr instr;
            instr.cmd = IRCmd::LEA_STRING;
            instr.imm = node.val; // string literal index
            instr.t = vid;
            func.instrPool.push_back(instr);
            return vid;
        }
        case ASTKind::Variable:
        {
            SymbolIdx symIdx = module.astSymMap[idx];
            Symbol& sym = module.getSymbol(symIdx);
            VRegID vid = func.newVRegVar(sym);
            return vid;
        }
        case ASTKind::FunctionCall:
        {
            SymbolIdx symIdx = module.astSymMap[idx];

            VRegID retVid = func.newVReg();
            IRInstr instr;
            instr.cmd = IRCmd::CALL;
            instr.imm = symIdx; // function symbol index
            instr.t = retVid;

            // prepare arguments
            for(size_t i = 0; i < node.args.size(); i++){
                VRegID argVid = genExpr(node.args[i]);
                instr.args.push_back(argVid);
            }

            func.instrPool.push_back(instr);

            return retVid;
        }
        case ASTKind::Switch:
        {
            VRegID retVal = func.newVReg();
            VRegID condVid = genExpr(node.cond);
            int32_t endLabel = func.newLabel();

            // condition check
            for(auto caseIdx : node.body){
                ASTNode caseNode = ps.getAST(caseIdx);
                // caseNode.lhs: case value
                // caseNode.rhs: case body

                // compare condition with case value
                VRegID caseValVid = genExpr(caseNode.lhs);
                VRegID cmpVid = func.newVReg();
                func.newIRInstr(IRCmd::EQUAL, condVid, caseValVid, cmpVid);

                // jump to next case if not equal
                IRInstr jz;
                jz.cmd = IRCmd::JZ;
                jz.s1 = cmpVid;
                jz.imm = func.newLabel();
                func.instrPool.push_back(jz);

                // generate case body
                VRegID caseRetVid = genExpr(caseNode.rhs);
                // move caseRetVid to retVal
                func.newIRInstr(IRCmd::MOV, caseRetVid, -1, retVal);

                // jump to end
                IRInstr jmpEnd;
                jmpEnd.cmd = IRCmd::JMP;
                jmpEnd.imm = endLabel;
                func.instrPool.push_back(jmpEnd);

                // next case label
                IRInstr nextCaseLabel;
                nextCaseLabel.cmd = IRCmd::LLABEL;
                nextCaseLabel.imm = jz.imm;
                func.instrPool.push_back(nextCaseLabel);
            }

            IRInstr jmpEnd;
            jmpEnd.cmd = IRCmd::LLABEL;
            jmpEnd.imm = endLabel;
            func.instrPool.push_back(jmpEnd);
            return retVal;
        }
        default:
            break;
    }

    VRegID lhs = genExpr(node.lhs);
    VRegID rhs = genExpr(node.rhs);
    
    switch(node.kind){
        case ASTKind::Add:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::ADD, lhs, rhs, t);
                return t;
            }
        case ASTKind::Sub:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::SUB, lhs, rhs, t);
                return t;
            }
        case ASTKind::Mul:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::MUL, lhs, rhs, t);
                return t;
            }
        case ASTKind::Div:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::DIV, lhs, rhs, t);
                return t;
            }
        case ASTKind::Mod:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::MOD, lhs, rhs, t);
                return t;
            }
        case ASTKind::LogicalOr:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::LOGICAL_OR, lhs, rhs, t);
                return t;
            }
        case ASTKind::LogicalAnd:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::LOGICAL_AND, lhs, rhs, t);
                return t;
            }
        case ASTKind::BitOr:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::BIT_OR, lhs, rhs, t);
                return t;
            }
        case ASTKind::BitXor:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::BIT_XOR, lhs, rhs, t);
                return t;
            }
        case ASTKind::BitAnd:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::BIT_AND, lhs, rhs, t);
                return t;
            }
        case ASTKind::Equal:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::EQUAL, lhs, rhs, t);
                return t;
            }
        case ASTKind::NotEqual:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::NEQUAL, lhs, rhs, t);
                return t;
            }
        case ASTKind::LessThan:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::LT, lhs, rhs, t);
                return t;
            }
        case ASTKind::LessEqual:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::LE, lhs, rhs, t);
                return t;
            }
        case ASTKind::LShift:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::LSHIFT, lhs, rhs, t);
                return t;
            }
        case ASTKind::RShift:
            {
                VRegID t = func.newVReg();
                func.newIRInstr(IRCmd::RSHIFT, lhs, rhs, t);
                return t;
            }
        default:
            break;
    }

    fprintf(stderr, "Unknown AST node kind in expression: %d\n", (uint32_t)node.kind);
    exit(1);
}

VRegID IRGenerator::genlvalue(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::Variable:
        {
            SymbolIdx symIdx = module.astSymMap[idx];
            Symbol& sym = module.getSymbol(symIdx);
            VRegID addrVid = func.newVReg();
            IRInstr addrInstr;
            addrInstr.cmd = IRCmd::FRAME_ADDR;
            addrInstr.t = addrVid;
            addrInstr.imm = sym.stackOffset;
            func.instrPool.push_back(addrInstr);
            return addrVid;
        }
        default:
            fprintf(stderr, "Invalid lvalue AST node kind: %d\n", (uint32_t)node.kind);
            exit(1);
    }
}
// ----------------------------------------------------------------
// IRFunc methods

void IRFunc::RegAlloc::computeUse(){
    size_t nV = f.vregs.size();
    lastUse.assign(nV, -1);
    for(size_t i = 0; i < f.instrPool.size(); i++) {
        const auto& ins = f.instrPool[i];
        auto mark = [&](VRegID v) {
            if(v >= 0 && (size_t)v < nV) {
                lastUse[v] = std::max(lastUse[v], (VRegID)i);
            }
        };
        mark(ins.s1);
        mark(ins.s2);
        mark(ins.t);
        for(auto arg : ins.args){
            mark(arg);
        }
    }
}

void IRFunc::RegAlloc::expireAt(size_t pos){
    for(size_t vid = 0; vid < f.vregs.size(); vid++){
        auto& vr = f.vregs[vid];
        if(vr.assigned != PhysReg::None     // assigned to physical register
            && lastUse[vid] >= 0            // used at least once
            && lastUse[vid] < (VRegID)pos)          // last used before current position  
        {
            freeRegs.push_back(vr.assigned);
            vr.assigned = PhysReg::None;
        }
    }
}

PhysReg IRFunc::RegAlloc::alloc(VRegID vid){
    auto& vr = f.getVReg(vid);
    if(vr.assigned != PhysReg::None) return vr.assigned;
    if(!freeRegs.empty()){
        PhysReg r = freeRegs.back();
        freeRegs.pop_back();
        vr.assigned = r;
        return r;
    }
    fprintf(stderr, "No free registers available for VReg %d\n", vid);
    exit(1);
}

void IRFunc::clean(){
    instrPool.clear();
    fname = "";
    labelCounter = 0;
}
void IRFunc::newIRInstr(const IRCmd cmd, VRegID s1, VRegID s2, VRegID t) {
    IRInstr instr;
    instr.cmd = cmd;
    instr.s1 = s1;
    instr.s2 = s2;
    instr.t = t;
    instrPool.push_back(instr);
}
int32_t IRFunc::newLabel(){
    return labelCounter++;
}
VRegID IRFunc::newVReg(){
    VReg vr;
    vregs.push_back(vr);
    return vregs.size() - 1;
}
VRegID IRFunc::newVRegNum(int32_t val){
    VRegID vid = newVReg();
    IRInstr isntr;
    isntr.cmd = IRCmd::MOV_IMM;
    isntr.t = vid;
    isntr.imm = val;
    instrPool.push_back(isntr);
    return vid;
}
VRegID IRFunc::newVRegVar(Symbol sym){
    
    VRegID addrVid = newVReg();
    IRInstr addrInstr;
    addrInstr.cmd = IRCmd::FRAME_ADDR;
    addrInstr.t = addrVid;
    addrInstr.imm = sym.stackOffset;
    instrPool.push_back(addrInstr);

    VRegID vid = newVReg();
    IRInstr instr;
    instr.cmd = IRCmd::LOAD;
    instr.t = vid;
    instr.s1 = addrVid;
    instrPool.push_back(instr);
    return vid;
}
VReg& IRFunc::getVReg(VRegID id){
    if(id < 0){
        fprintf(stderr, "VReg is not used.\n");
        exit(1);
    }
    if((size_t)id >= vregs.size()){
        fprintf(stderr, "Invalid VRegID: %d\n", id);
        exit(1);
    }
    return vregs[id];
}

// ----------------------------------------------------------------
// IRModule methods

SymbolIdx IRModule::findSymbol(const std::string& name, ScopeIdx idx){
    if(idx < 0 || (size_t)idx >= scopes.size()){
        return -1; // not found
    }
    Scope& sc = scopes[idx];

    SymbolIdx symIdx = sc.findSymbol(name);
    if(symIdx != -1){
        return symIdx;
    }

    if(sc.parent != -1){
        return findSymbol(name, sc.parent);
    }

    return -1; // not found
}

void IRModule::scopeIn(){
    ScopeIdx parentIdx = curScope;
    scopes.push_back(Scope(parentIdx));
    curScope = scopes.size() - 1;

    // when into function scope
    if(parentIdx == globalScope){
        funcScope = curScope;
    }
};

void IRModule::scopeOut(){
    Scope& sc = scopes[curScope];
    if(sc.parent == -1){
        fprintf(stderr, "Cannot scope out from global scope.\n");
        exit(1);
    }

    curScope = sc.parent;

    // when out of function scope
    if(curScope == globalScope){
        funcScope = -1;
    }
}

int32_t IRModule::addString(std::string data){
    int32_t id = stringLiterals.size();
    StringLiteralData sld;
    sld.id = id;
    sld.str = data;
    stringLiterals.push_back(sld);
    return id;
}

SymbolIdx IRModule::insertSymbol(const Symbol& sym){
    // check duplication
    Scope& sc = scopes[curScope];
    if(sc.symbols.find(sym.name) != sc.symbols.end()){
        fprintf(stderr, "Symbol already exists in current scope: %s\n", sym.name.c_str());
        exit(1);
    }

    // add to pool
    symbolPool.push_back(sym);
    SymbolIdx symIdx = symbolPool.size() - 1;

    // add to scope
    sc.insertSymbol(sym.name, symIdx);

    Symbol& s = symbolPool[symIdx];
    currentStackSize += 8; // assuming 8 bytes per variable
    s.stackOffset = currentStackSize;

    return symIdx;
}

Symbol& IRModule::getSymbol(SymbolIdx idx){
    if(idx < 0 || (size_t)idx >= symbolPool.size()){
        fprintf(stderr, "Invalid SymbolIdx: %d\n", idx);
        exit(1);
    }
    return symbolPool[idx];
}

void IRModule::outputSymbols(std::string dir, std::string module){
    if(dir == ""){
        dir = ".";
    }

    FILE *fp;
    std::string filename = dir + "/" + module + ".tnlib";
    fp = fopen(filename.c_str(), "w");
    if(!fp){
        fprintf(stderr, "Cannot open file: %s\n", filename.c_str());
        exit(1);
    }

    fprintf(fp, "tnlib 1\n");
    fprintf(fp, "module %s\n", module.c_str());
    for(size_t i = 0; i < symbolPool.size(); i++){
        const auto& sym = symbolPool[i];
        if(sym.isPub() == false){
            continue; // skip non-public symbols
        }
        if(sym.kind == SymbolKind::Function){
            fprintf(fp, "fn %s(", sym.name.c_str());
            for(size_t j = 0; j < sym.params.size(); j++){
                const auto& paramSym = getSymbol(sym.params[j]);
                fprintf(fp, "%s", paramSym.name.c_str());
                if(j + 1 < sym.params.size()){
                    fprintf(fp, ", ");  
                }
            }
            fprintf(fp, ");\n");
        }
    }
    fprintf(fp, "end\n");
    fclose(fp);
}

void IRModule::printSymbols(){
    for(size_t i = 0; i < symbolPool.size(); i++){
        const auto& sym = symbolPool[i];
        std::cout << "Symbol[" << i << "]: " << sym.name << ", mut=" << sym.isMut() << "\n";
    }
}
