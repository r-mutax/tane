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
    module.currentStackSize = 0;
    module.scopeIn();
    bindStmt(idx);
    module.scopeOut();

    module.funcSem[idx].localBytes = module.currentStackSize;
}

void IRGenerator::bindStmt(ASTIdx idx){
    ASTNode node = ps.getAST(idx);

    switch(node.kind){
        case ASTKind::CompoundStmt: {
            module.scopeIn();
            for(auto stmtIdx : node.body){
                bindStmt(stmtIdx);
            }
            module.scopeOut();
            break;
        }
        case ASTKind::VarDecl: {
            Symbol sym;
            sym.name = node.name;
            sym.isMut = node.is_mut;

            module.insertSymbol(sym);
            break;
        }
        default:
            break;
    }
}

IRFunc IRGenerator::genFunc(ASTIdx idx){
    func.clean();
    func.fname = "main";

    FuncSem fs = module.funcSem[idx];
    func.localStackSize = fs.localBytes;

    genStmt(idx);

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
        case ASTKind::VarDecl: {
            // Currently, do nothing for variable declarations
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