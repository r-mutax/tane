#include "tane.hpp"

IRModule& IRGenerator::run(){

    module.funcPool.clear();
    // Bind the translation unit
    bindTU(root);

    // generation of main function
    ASTNode rootNode = ps.getAST(root);
    for(auto stmtIdx : rootNode.body){
        module.funcPool.push_back(genFunc(stmtIdx));
    }

    return module;
}

void IRGenerator::bindTU(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    for(auto funcIdx : node.body){
        bindFunc(funcIdx);
    }
}

void IRGenerator::bindFunc(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    Symbol sym;
    sym.name = node.name;
    sym.isMut = false; // functions are not mutable
    sym.kind = SymbolKind::Function;
    module.insertSymbol(sym);

    module.currentStackSize = 0;
    module.scopeIn();

    // Add function parameters to symbol table
    std::vector<SymbolIdx> paramSyms;
    for(auto paramIdx : node.params){
        ASTNode paramNode = ps.getAST(paramIdx);
        Symbol sym;
        sym.name = paramNode.name;
        sym.isMut = true;
        sym.kind = SymbolKind::Variable;
        SymbolIdx symidx = module.insertSymbol(sym); 
        paramSyms.push_back(symidx);  
    }
    FuncSem& fs = module.funcSem[idx];
    fs.params = paramSyms;

    bindStmt(node.body[0]);  // function body
    module.scopeOut();

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
            sym.isMut = node.is_mut;

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
    ASTNode node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::Num:
            // nothing to do
            return;
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
            fprintf(stderr, "Unknown AST node kind: %d\n", (uint32_t)node.kind);
            exit(1);
    }
}

VRegID IRGenerator::genExpr(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::Num:
            {
                return func.newVRegNum(node.val);
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
            Symbol& sym = module.getSymbol(symIdx);

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

