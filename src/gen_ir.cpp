#include "tane.hpp"

IRModule& IRGenerator::run(){

    module.funcPool.clear();
    module.funcPool.push_back(genFunc());

    return module;
}

IRFunc IRGenerator::genFunc(void){
    func.clean();
    func.fname = "main";

    genStmt(root);

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
                return func.newVRegNum(node.data.val);
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