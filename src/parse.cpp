#include "tane.hpp"

ASTIdx Parser::parseFile() {
    ts.reset();

    ASTIdx tu = newNode(ASTKind::TranslationUnit, 0, 0);

    while(!ts.peekKind(TokenKind::Eof)){
        ASTIdx func = functionDef();

        ASTNode& tuNode = getAST(tu);
        tuNode.body.push_back(func);
    }

    return tu;
}

ASTIdx Parser::functionDef(){
    ts.expect(TokenKind::Fn);

    TokenIdx ident = ts.expectIdent();
    Token t = ts.getToken(ident);
    std::string fname(t.pos, t.len);

    ts.expect(TokenKind::LParen);
    ts.expect(TokenKind::RParen);

    ts.expect(TokenKind::LBrace);
    ASTIdx body = compoundStmt();

    ASTIdx n = newNode(ASTKind::Function, 0, 0);
    ASTNode& node = getAST(n);
    node.name = fname;
    node.body.push_back(body);
    return n;
}

ASTIdx Parser::compoundStmt(){
    ASTIdx n = newNode(ASTKind::CompoundStmt, 0, 0);
    
    std::vector<ASTIdx> body;   
    while(!ts.consume(TokenKind::RBrace)){
        body.push_back(stmt());
    }

    ASTNode& node = getAST(n);
    node.body = body;
    return n;
}

ASTIdx Parser::stmt(){
    if(ts.consume(TokenKind::Return)){
        ASTIdx n = newNode(ASTKind::Return, expr(), 0);
        ts.expect(TokenKind::Semicolon);
        return n;
    } else if(ts.consume(TokenKind::If)){
        ASTIdx cond = expr();
        
        ts.expect(TokenKind::LBrace);
        ASTIdx thenBr = compoundStmt();

        ASTIdx elseBr = -1;
        if(ts.peekKind(TokenKind::Else)){
            ts.consume(TokenKind::Else);
            if(ts.peekKind(TokenKind::If)){
                // else if
                elseBr = stmt();
            } else {
                // else
                ts.expect(TokenKind::LBrace);
                elseBr = compoundStmt();
            }
        }

        ASTIdx n = newNode(ASTKind::If, cond, thenBr);
        ASTNode& node = getAST(n);
        node.cond = cond;
        node.thenBr = thenBr;
        node.elseBr = elseBr;
        return n;
    } else if(ts.consume(TokenKind::While)){
        ASTIdx cond = expr();

        ts.expect(TokenKind::LBrace);
        ASTIdx body = compoundStmt();

        ASTIdx n = newNode(ASTKind::While, cond, body);
        ASTNode& node = getAST(n);
        node.cond = cond;
        node.body.push_back(body);
        return n;
    } else if(ts.consume(TokenKind::Let)){
        bool is_mut = false;
        if(ts.consume(TokenKind::Mut)){
            is_mut = true;
        }
        TokenIdx ident = ts.expectIdent();

        ASTIdx n = newNode(ASTKind::VarDecl, 0, 0);
        ASTNode& node = getAST(n);

        Token t = ts.getToken(ident);
        node.name = std::string(t.pos, t.len);
        node.is_mut = is_mut;
        ts.expect(TokenKind::Semicolon);
        return n;
    } else if(ts.consume(TokenKind::LBrace)){
        return compoundStmt();
    } else {
        if(ts.peekKind(TokenKind::Ident) && ts.peekKind(TokenKind::Equal, 1)){
            // assignment statement
            TokenIdx ident = ts.expectIdent();
            ts.expect(TokenKind::Equal);

            ASTIdx n = newNode(ASTKind::Variable, 0, 0);
            ASTNode& node = getAST(n);
            Token t = ts.getToken(ident);
            node.name = std::string(t.pos, t.len);

            ASTIdx rhs = expr();
            ASTIdx assignNode = newNode(ASTKind::Assign, n, rhs);
            ts.expect(TokenKind::Semicolon);
            return assignNode;
        }

        // expr statement
        ASTIdx n = expr();
        ts.expect(TokenKind::Semicolon);
        return n;
    }
}

ASTIdx Parser::expr(){
    return logical_or();
}

ASTIdx Parser::logical_or(){
    ASTIdx lhs = logical_and();

    while(true){
        if(ts.consume(TokenKind::OrOr)){
            lhs = newNode(ASTKind::LogicalOr, lhs, logical_and());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::logical_and(){
    ASTIdx lhs = bitwise_or();

    while(true){
        if(ts.consume(TokenKind::AndAnd)){
            lhs = newNode(ASTKind::LogicalAnd, lhs, bitwise_or());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::bitwise_or(){
    ASTIdx lhs = bitwise_xor();

    while(true){
        if(ts.consume(TokenKind::Or)){
            lhs = newNode(ASTKind::BitOr, lhs, bitwise_xor());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::bitwise_xor(){
    ASTIdx lhs = bitwise_and();

    while(true){
        if(ts.consume(TokenKind::Hat)){
            lhs = newNode(ASTKind::BitXor, lhs, bitwise_and());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::bitwise_and(){
    ASTIdx lhs = equality();

    while(true){
        if(ts.consume(TokenKind::And)){
            lhs = newNode(ASTKind::BitAnd, lhs, equality());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::equality(){
    ASTIdx lhs = relational();

    while(true){
        if(ts.consume(TokenKind::EqualEqual)){
            lhs = newNode(ASTKind::Equal, lhs, relational());
            continue;
        }
        if(ts.consume(TokenKind::NotEqual)){
            lhs = newNode(ASTKind::NotEqual, lhs, relational());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::relational(){
    ASTIdx lhs = shift();

    while(true){
        if(ts.consume(TokenKind::LessThan)){
            lhs = newNode(ASTKind::LessThan, lhs, shift());
            continue;
        }
        if(ts.consume(TokenKind::LessEqual)){
            lhs = newNode(ASTKind::LessEqual, lhs, shift());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::shift(){
    ASTIdx lhs = add();

    while(true){
        if(ts.consume(TokenKind::LShift)){
            lhs = newNode(ASTKind::LShift, lhs, add());
            continue;
        }
        if(ts.consume(TokenKind::RShift)){
            lhs = newNode(ASTKind::RShift, lhs, add());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::add(){
    ASTIdx lhs = mul();

    while(true){
        if(ts.consume(TokenKind::Add)){
            lhs = newNode(ASTKind::Add, lhs, mul());
            continue;
        }
        if(ts.consume(TokenKind::Sub)){
            lhs = newNode(ASTKind::Sub, lhs, mul());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::mul(){
    ASTIdx lhs = unary();

    while(true){
        if(ts.consume(TokenKind::Mul)){
            lhs = newNode(ASTKind::Mul, lhs, unary());
            continue;
        }
        if(ts.consume(TokenKind::Div)){
            lhs = newNode(ASTKind::Div, lhs, unary());
            continue;
        }
        if(ts.consume(TokenKind::Mod)){
            lhs = newNode(ASTKind::Mod, lhs, unary());
            continue;
        }
        return lhs;
    }
}

ASTIdx Parser::unary(){
    if(ts.consume(TokenKind::Add)){
        return primary();
    }
    if(ts.consume(TokenKind::Sub)){
        return newNode(ASTKind::Sub, newNodeNum(0), primary());
    }
    return primary();
}

ASTIdx Parser::primary(){
    if(ts.consume(TokenKind::LParen)){
        ASTIdx n = expr();
        ts.expect(TokenKind::RParen);
        return n;
    }

    if(auto idxOpt = ts.consumeIdent()){
        TokenIdx idx = *idxOpt;
        Token t = ts.getToken(idx);
        ASTIdx n = newNode(ASTKind::Variable, 0, 0);
        ASTNode& node = getAST(n);
        node.name = std::string(t.pos, t.len);
        return n;
    }

    if(ts.consume(TokenKind::Switch)){
        ASTIdx cond = expr();

        ASTIdx n = newNode(ASTKind::Switch, 0, 0);
        getAST(n).cond = cond;
        ts.expect(TokenKind::LBrace);
        while(!ts.consume(TokenKind::RBrace)){
            ASTIdx caseExpr = expr();
            ts.expect(TokenKind::EqualArrow);
            ASTIdx caseBody = expr();

            ASTIdx caseNode = newNode(ASTKind::Case, caseExpr, caseBody);
            getAST(n).body.push_back(caseNode);
            if(ts.peekKind(TokenKind::RBrace, 1) == false){
                ts.expect(TokenKind::Comma);
            } else {
                ts.consume(TokenKind::Comma);
            }
        }
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