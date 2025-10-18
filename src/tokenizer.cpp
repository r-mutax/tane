#include "tane.hpp"

Tokenizer::TokenStream& Tokenizer::scan(char* p){
    ts.clear();

    bool continue_flg = true;

    while(continue_flg){
        char c = *p;

        switch(c){
            case 0:
                ts.addToken(TK_EOF, p);
                continue_flg = false;
                break;            
            case '+':
                ts.addToken(TK_ADD, p++);
                break;
            case '-':
                ts.addToken(TK_SUB, p++);
                break;
            case '*':
                ts.addToken(TK_MUL, p++);
                break;
            case '/':
                ts.addToken(TK_DIV, p++);
                break;
            case '%':
                ts.addToken(TK_MOD, p++);
                break;
            case '=':
                if(*(p + 1) == '='){
                    ts.addToken(TK_EQUAL, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    fprintf(stderr, "Invalid token: %s\n", p);
                    exit(1);
                }
            case '!':
                if(*(p + 1) == '='){
                    ts.addToken(TK_NOT_EQUAL, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    fprintf(stderr, "Invalid token: %s\n", p);
                    exit(1);
                }
            case '<':
                if(*(p + 1) == '='){
                    ts.addToken(TK_LESS_EQUAL, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else if(*(p + 1) == '<'){
                    ts.addToken(TK_LSHIFT, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    ts.addToken(TK_LESS_THAN, p++);
                    break;
                }
            case '>':
                if(*(p + 1) == '>'){
                    ts.addToken(TK_RSHIFT, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                }
                break;
            case '|':
                if(*(p + 1) == '|'){
                    ts.addToken(TK_OR_OR, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    ts.addToken(TK_OR, p++);
                    break;
                }
            case '^':
                ts.addToken(TK_HAT, p++);
                break;
            case '&':
                if(*(p + 1) == '&'){
                    ts.addToken(TK_AND_AND, p);
                    ts.getTop().len = 2;
                    p += 2;
                    break;
                } else {
                    ts.addToken(TK_AND, p++);
                    break;
                }
            case '(':
                ts.addToken(TK_L_PAREN, p++);
                break;
            case ')':
                ts.addToken(TK_R_PAREN, p++);
                break;
            case '{':
                ts.addToken(TK_L_BRACE, p++);
                break;
            case '}':
                ts.addToken(TK_R_BRACE, p++);
                break;
            case ';':
                ts.addToken(TK_SEMICOLON, p++);
                break;
            default:
                if(isdigit(c)){
                    char* q = p;
                    int32_t val = strtol(p, &p, 10);

                    ts.addToken(TK_NUM, q);
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
    return TK_IDENT;
}

bool Tokenizer::is_ident1(char c){
    return isalpha(c) || c == '_';
}

bool Tokenizer::is_ident2(char c){
    return is_ident1(c) || isdigit(c);
}

Tokenizer::Tokenizer(){
    // Initialize keyword map
    keyword_map["return"] = TK_RETURN;
}

bool Tokenizer::TokenStream::consume(TokenKind kind){
    if(idx == tokens.size()){
        return false;
    }
    if(tokens[idx].kind != kind){
        return false;
    }

    idx++;
    return true;
}

void Tokenizer::TokenStream::expect(TokenKind kind){
    if(idx == tokens.size()){
        fprintf(stderr, "Unexpected end of input\n");
        exit(1);
    }

    if(tokens[idx].kind != kind){
        fprintf(stderr, "Unexpected token: %d\n", tokens[idx].kind);
        exit(1);
    }

    idx++;
    return;
}

int32_t Tokenizer::TokenStream::expectNum(){
    if(idx == tokens.size()){
        fprintf(stderr, "Unexpected end of input\n");
        exit(1);
    }

    if(tokens[idx].kind != TK_NUM){
        fprintf(stderr, "Unexpected token: %d\n", tokens[idx].kind);
        exit(1);
    }

    return tokens[idx++].val;
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
        case TK_NUM:
            printf("TK_NUM");
            break;
        case TK_ADD:
            printf("TK_ADD");
            break;
        case TK_SUB:
            printf("TK_SUB");
            break;
        case TK_MUL:
            printf("TK_MUL");
            break;
        case TK_DIV:
            printf("TK_DIV");
            break;
        case TK_MOD:
            printf("TK_MOD");
            break;
        case TK_L_PAREN:
            printf("TK_L_PAREN");
            break;
        case TK_R_PAREN:
            printf("TK_R_PAREN");
            break;
        case TK_RETURN:
            printf("TK_RETURN");
            break;
        case TK_SEMICOLON:
            printf("TK_SEMICOLON");
            break;
        case TK_EOF:
            printf("TK_EOF");
            break;
        default:
            printf("Unknown TokenKind: %d", kind);
            break;
    }
}