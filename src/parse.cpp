#include "tane.hpp"

ASTIdx Parser::parseFile() {
    ts.reset();
    return stmt();
}

ASTIdx Parser::stmt(){
    if(ts.consume(TK_RETURN)){
        ASTIdx n = newNode(ASTKind::Return, expr(), 0);
        ts.expect(TK_SEMICOLON);
        return n;
    } else {
        // expr statement
        ASTIdx n = expr();
        ts.expect(TK_SEMICOLON);
        return n;
    }
}

ASTIdx Parser::expr(){
    return primary();
}

ASTIdx Parser::primary(){
    int32_t n = ts.expectNum();
    return newNodeNum(n);
}

ASTIdx Parser::newNode(ASTKind kind, ASTIdx lhs, ASTIdx rhs) {
    ASTNode node;
    node.kind = kind;
    node.lhs = lhs;
    node.rhs = rhs;
    
    nodes.push_back(node);
    return nodes.size() - 1;
}

ASTIdx Parser::newNodeNum(int32_t val) {
    ASTIdx idx = newNode(ASTKind::Num, 0, 0);
    nodes[idx].data.val = val;
    return idx;
}


// for debugging
void Parser::printAST(ASTIdx idx) {

    printf("AST:\n");
    printStmt(idx);
}

void Parser::printStmt(ASTIdx idx) {
    depth++;
    ASTNode node = getAST(idx);
    switch(node.kind) {
        case ASTKind::Return:
            printAST(node, depth);
            printExpr(node.lhs);
            break;
        default:
            printExpr(idx);
            break;
    }
    depth--;
}

void Parser::printExpr(ASTIdx idx) {
    depth++;
    ASTNode node = getAST(idx);
    printAST(node, depth);
    depth--;
}

void Parser::printAST(const ASTNode& node, int32_t depth) const {
    for(int32_t i = 0; i < depth; i++) {
        printf("  ");
    }
    switch(node.kind) {
        case ASTKind::Num:
            printf("ND_NUM: %d\n", node.data.val);
            break;
        case ASTKind::Add:
            printf("ND_ADD\n");
            break;
        case ASTKind::Sub:
            printf("ND_SUB\n");
            break;
        case ASTKind::Mul:
            printf("ND_MUL\n");
            break;
        case ASTKind::Div:
            printf("ND_DIV\n");
            break;
        case ASTKind::Return:
            printf("ND_RETURN\n");
            break;
        default:
            printf("Unknown AST node kind: %d\n", (int32_t)node.kind);
            exit(1);
    }
}