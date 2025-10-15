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
            fprintf(stderr, "Unknown AST node kind: %d\n", (unsigned int)node.kind);
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
    fprintf(stderr, "%d\n", idx);
    fprintf(stderr, "Unknown AST node kind in expression: %d\n", (unsigned int)node.kind);
    exit(1);
}