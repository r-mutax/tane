#include "tane.hpp"

Tokenizer::TokenStream& Tokenizer::scan(char* p){
    ts.clear();

    bool continue_flg = true;

    while(continue_flg){
        char c = *p;

        switch(c){
            case 0:
                ts.addToken(TokenKind::Eof, p);
                continue_flg = false;
                break;            
            case '+':
                ts.addToken(TokenKind::Add, p++);
                break;
            case '-':
                ts.addToken(TokenKind::Sub, p++);
                break;
            case '*':
                ts.addToken(TokenKind::Mul, p++);
                break;
            case '/':
                ts.addToken(TokenKind::Div, p++);
                break;
            case '%':
                ts.addToken(TokenKind::Mod, p++);
                break;
            case '=':
                if(*(p + 1) == '='){
                    ts.addToken(TokenKind::EqualEqual, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else if(*(p + 1) == '>'){
                    ts.addToken(TokenKind::EqualArrow, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    ts.addToken(TokenKind::Equal, p++);
                    break;
                }
            case '!':
                if(*(p + 1) == '='){
                    ts.addToken(TokenKind::NotEqual, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    fprintf(stderr, "Invalid token: %s\n", p);
                    exit(1);
                }
            case '<':
                if(*(p + 1) == '='){
                    ts.addToken(TokenKind::LessEqual, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else if(*(p + 1) == '<'){
                    ts.addToken(TokenKind::LShift, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    ts.addToken(TokenKind::LessThan, p++);
                    break;
                }
            case '>':
                if(*(p + 1) == '>'){
                    ts.addToken(TokenKind::RShift, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                }
                break;
            case '|':
                if(*(p + 1) == '|'){
                    ts.addToken(TokenKind::OrOr, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    ts.addToken(TokenKind::Or, p++);
                    break;
                }
            case '^':
                ts.addToken(TokenKind::Hat, p++);
                break;
            case '&':
                if(*(p + 1) == '&'){
                    ts.addToken(TokenKind::AndAnd, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    ts.addToken(TokenKind::And, p++);
                    break;
                }
            case '(':
                ts.addToken(TokenKind::LParen, p++);
                break;
            case ')':
                ts.addToken(TokenKind::RParen, p++);
                break;
            case '{':
                ts.addToken(TokenKind::LBrace, p++);
                break;
            case '}':
                ts.addToken(TokenKind::RBrace, p++);
                break;
            case ';':
                ts.addToken(TokenKind::Semicolon, p++);
                break;
            case ',':
                ts.addToken(TokenKind::Comma, p++);
                break;
            default:
                if(isdigit(c)){
                    char* q = p;
                    int32_t val = strtol(p, &p, 10);

                    ts.addToken(TokenKind::Num, q);
                    ts.getTop().val = val;
                } else if(isspace(c)){
                    p++;
                } else if(is_ident1(c)){
                    char* q = p;
                    p++;
                    while(is_ident2(*p)) p++;
                    ts.addToken(checkKeyword(q, p - q), q);
                    ts.getTop().len = p - q;
                } else {
                    fprintf(stderr, "Cannot tokenize: %s\n", p);
                    exit(1);
                }                
                break;
        }
    }
    return ts;
}

TokenKind Tokenizer::checkKeyword(char* start, uint32_t len){
    if(keyword_map.find(std::string(start, len)) != keyword_map.end()){
        return keyword_map[std::string(start, len)];
    }

    // Not a keyword, so it must be an identifier
    return TokenKind::Ident;
}

bool Tokenizer::is_ident1(char c){
    return isalpha(c) || c == '_';
}

bool Tokenizer::is_ident2(char c){
    return is_ident1(c) || isdigit(c);
}

Tokenizer::Tokenizer(){
    // Initialize keyword map
    keyword_map["return"] = TokenKind::Return;
    keyword_map["let"] = TokenKind::Let;
    keyword_map["mut"] = TokenKind::Mut;
    keyword_map["if"] = TokenKind::If;
    keyword_map["else"] = TokenKind::Else;
    keyword_map["while"] = TokenKind::While;
    keyword_map["switch"] = TokenKind::Switch;
    keyword_map["fn"] = TokenKind::Fn;
}

bool Tokenizer::TokenStream::consume(TokenKind kind){
    if(idx >= tokens.size()){
        return false;
    }
    if(tokens[idx].kind != kind){
        return false;
    }

    idx++;
    return true;
}

void Tokenizer::TokenStream::expect(TokenKind kind){
    if(idx >= tokens.size()){
        fprintf(stderr, "Unexpected end of input\n");
        exit(1);
    }

    if(tokens[idx].kind != kind){
        fprintf(stderr, "Unexpected token: %ud\n", (unsigned int)tokens[idx].kind);
        exit(1);
    }

    idx++;
    return;
}

int32_t Tokenizer::TokenStream::expectNum(){
    if(idx >= tokens.size()){
        fprintf(stderr, "Unexpected end of input\n");
        exit(1);
    }

    if(tokens[idx].kind != TokenKind::Num){
        fprintf(stderr, "Unexpected token: %d\n", static_cast<int>(tokens[idx].kind));
        exit(1);
    }

    return tokens[idx++].val;
}

TokenIdx Tokenizer::TokenStream::expectIdent(){
    if(idx >= tokens.size()){
        fprintf(stderr, "Unexpected end of input\n");
        exit(1);
    }

    if(tokens[idx].kind != TokenKind::Ident){
        fprintf(stderr, "Unexpected token: %d\n", static_cast<int>(tokens[idx].kind));
        exit(1);
    }

    return idx++;
}

std::optional<TokenIdx> Tokenizer::TokenStream::consumeIdent(){
    if(idx >= tokens.size()){
        return std::nullopt;
    }
    if(tokens[idx].kind != TokenKind::Ident){
        return std::nullopt;
    }
    return idx++;
}


// For debugging

/// Print the tokens in the linked list
void Tokenizer::printTokens(){
    for(auto it : ts.tokens) {
        printTokenKind(it.kind); 
        printf("\n");
    }
}

/// Print a single TokenKind
void Tokenizer::printTokenKind(TokenKind kind){
    switch(kind){
        case TokenKind::Num:
            printf("Num");
            break;
        case TokenKind::Add:
            printf("Add");
            break;
        case TokenKind::Sub:
            printf("Sub");
            break;
        case TokenKind::Mul:
            printf("Mul");
            break;
        case TokenKind::Div:
            printf("Div");
            break;
        case TokenKind::Mod:
            printf("Mod");
            break;
        case TokenKind::LParen:
            printf("LParen");
            break;
        case TokenKind::RParen:
            printf("RParen");
            break;
        case TokenKind::Return:
            printf("Return");
            break;
        case TokenKind::Semicolon:
            printf("Semicolon");
            break;
        case TokenKind::Eof:
            printf("Eof");
            break;
        default:
            printf("Unknown TokenKind: %d", static_cast<int>(kind));
            break;
    }
}