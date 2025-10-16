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
    return relational();
}

ASTIdx Parser::relational(){
    ASTIdx lhs = add();

    while(true){
        if(ts.consume(TK_EQUAL)){
            lhs = newNode(ASTKind::Equal, lhs, add());
            continue;
        }
        if(ts.consume(TK_NOT_EQUAL)){
            lhs = newNode(ASTKind::NotEqual, lhs, add());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::add(){
    ASTIdx lhs = mul();

    while(true){
        if(ts.consume(TK_ADD)){
            lhs = newNode(ASTKind::Add, lhs, mul());
            continue;
        }
        if(ts.consume(TK_SUB)){
            lhs = newNode(ASTKind::Sub, lhs, mul());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::mul(){
    ASTIdx lhs = unary();

    while(true){
        if(ts.consume(TK_MUL)){
            lhs = newNode(ASTKind::Mul, lhs, unary());
            continue;
        }
        if(ts.consume(TK_DIV)){
            lhs = newNode(ASTKind::Div, lhs, unary());
            continue;
        }
        if(ts.consume(TK_MOD)){
            lhs = newNode(ASTKind::Mod, lhs, unary());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::unary(){
    if(ts.consume(TK_ADD)){
        return primary();
    }
    if(ts.consume(TK_SUB)){
        return newNode(ASTKind::Sub, newNodeNum(0), primary());
    }
    return primary();
}

ASTIdx Parser::primary(){
    if(ts.consume(TK_L_PAREN)){
        ASTIdx n = expr();
        ts.expect(TK_R_PAREN);
        return n;
    }

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
        case ASTKind::Mod:
            printf("ND_MOD\n");
            break;
        case ASTKind::Return:
            printf("ND_RETURN\n");
            break;
        default:
            printf("Unknown AST node kind: %d\n", (int32_t)node.kind);
            exit(1);
    }
}