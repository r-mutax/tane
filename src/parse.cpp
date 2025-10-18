#include "tane.hpp"

ASTIdx Parser::parseFile() {
    ts.reset();

    ASTIdx tu = newNode(ASTKind::TranslationUnit, 0, 0);

    ts.expect(TK_L_BRACE);
    ASTIdx cs = compoundStmt();

    ASTNode& tuNode = getAST(tu);
    tuNode.body.push_back(cs);

    return tu;
}

ASTIdx Parser::compoundStmt(){
    ASTIdx n = newNode(ASTKind::CompoundStmt, 0, 0);
    
    std::vector<ASTIdx> body;   
    while(!ts.consume(TK_R_BRACE)){
        body.push_back(stmt());
    }

    ASTNode& node = getAST(n);
    node.body = body;
    return n;
}

ASTIdx Parser::stmt(){
    if(ts.consume(TK_RETURN)){
        ASTIdx n = newNode(ASTKind::Return, expr(), 0);
        ts.expect(TK_SEMICOLON);
        return n;
    } else if(ts.consume(TK_LET)){
        bool is_mut = false;
        if(ts.consume(TK_MUT)){
            is_mut = true;
        }
        TokenIdx ident = ts.expectIdent();

        ASTIdx n = newNode(ASTKind::VarDecl, 0, 0);
        ASTNode& node = getAST(n);

        Token t = ts.getToken(ident);
        node.name = std::string(t.pos, t.len);
        node.is_mut = is_mut;
        ts.expect(TK_SEMICOLON);
        return n;
    } else if(ts.consume(TK_L_BRACE)){
        return compoundStmt();
    } else {
        // expr statement
        ASTIdx n = expr();
        ts.expect(TK_SEMICOLON);
        return n;
    }
}

ASTIdx Parser::expr(){
    return logical_or();
}

ASTIdx Parser::logical_or(){
    ASTIdx lhs = logical_and();

    while(true){
        if(ts.consume(TK_OR_OR)){
            lhs = newNode(ASTKind::LogicalOr, lhs, logical_and());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::logical_and(){
    ASTIdx lhs = bitwise_or();

    while(true){
        if(ts.consume(TK_AND_AND)){
            lhs = newNode(ASTKind::LogicalAnd, lhs, bitwise_or());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::bitwise_or(){
    ASTIdx lhs = bitwise_xor();

    while(true){
        if(ts.consume(TK_OR)){
            lhs = newNode(ASTKind::BitOr, lhs, bitwise_xor());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::bitwise_xor(){
    ASTIdx lhs = bitwise_and();

    while(true){
        if(ts.consume(TK_HAT)){
            lhs = newNode(ASTKind::BitXor, lhs, bitwise_and());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::bitwise_and(){
    ASTIdx lhs = equality();

    while(true){
        if(ts.consume(TK_AND)){
            lhs = newNode(ASTKind::BitAnd, lhs, equality());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::equality(){
    ASTIdx lhs = relational();

    while(true){
        if(ts.consume(TK_EQUAL)){
            lhs = newNode(ASTKind::Equal, lhs, relational());
            continue;
        }
        if(ts.consume(TK_NOT_EQUAL)){
            lhs = newNode(ASTKind::NotEqual, lhs, relational());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::relational(){
    ASTIdx lhs = shift();

    while(true){
        if(ts.consume(TK_LESS_THAN)){
            lhs = newNode(ASTKind::LessThan, lhs, shift());
            continue;
        }
        if(ts.consume(TK_LESS_EQUAL)){
            lhs = newNode(ASTKind::LessEqual, lhs, shift());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::shift(){
    ASTIdx lhs = add();

    while(true){
        if(ts.consume(TK_LSHIFT)){
            lhs = newNode(ASTKind::LShift, lhs, add());
            continue;
        }
        if(ts.consume(TK_RSHIFT)){
            lhs = newNode(ASTKind::RShift, lhs, add());
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
    nodes[idx].val = val;
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
            printf("ND_NUM: %d\n", node.val);
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