#include "tane.h"

static Token* new_token(TokenKind kind, Token* cur, char* pos);
static bool is_ident1(char c);
static bool is_ident2(char c);
static TokenKind check_keyword(char* start, unsigned int len);

Token* tokenize(char* p) {
    Token head = { 0 };
    Token* cur = &head;

    bool continue_flg = true;

    while(continue_flg){
        char c = *p;

        switch(c){
            case 0:
                cur = new_token(TK_EOF, cur, p++);
                continue_flg = false;
                break;            
            case '+':
                cur = new_token(TK_ADD, cur, p++);
                break;
            case '-':
                cur = new_token(TK_SUB, cur, p++);
                break;
            case '*':
                cur = new_token(TK_MUL, cur, p++);
                break;
            case '/':
                cur = new_token(TK_DIV, cur, p++);
                break;
            case '(':
                cur = new_token(TK_L_PAREN, cur, p++);
                break;
            case ')':
                cur = new_token(TK_R_PAREN, cur, p++);
                break;
            case ';':
                cur = new_token(TK_SEMICOLON, cur, p++);
                break;
            default:
                if(isdigit(c)){
                    char* q = p;
                    int val = strtol(p, &p, 10);

                    cur = new_token(TK_NUM, cur, q);
                    cur->val = val;
                } else if(isspace(c)){
                    p++;
                } else if(is_ident1(c)){
                    char* q = p;
                    p++;
                    while(is_ident2(*p)) p++;
                    cur = new_token(check_keyword(q, p - q), cur, q);
                    cur->len = p - q;
                } else {
                    fprintf(stderr, "Cannot tokenize: %s\n", p);
                    exit(1);
                }                
                break;
        }
    }

    return head.next;
}

static Token* new_token(TokenKind kind, Token* cur, char* pos){
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->pos = pos;
    cur->next = tok;
    return tok;
}

static bool is_ident1(char c){
    return isalpha(c) || c == '_';
}

static bool is_ident2(char c){
    return is_ident1(c) || isdigit(c);
}

static TokenKind check_keyword(char* start, unsigned int len){
    for(unsigned int i = 0; i < sizeof(keyword_map) / sizeof(KEYWORD_MAP); i++){
        if(len == strlen(keyword_map[i].keyword)
                && (!memcmp(start, keyword_map[i].keyword, len))){
            return keyword_map[i].kind;
        }
    }

    // keyword_mapになかった場合、トークンは識別子
    return TK_IDENT;
}

// For debugging
void printTokenKind(TokenKind kind);

/// Print the tokens in the linked list
void print_tokens(Token* head){
    Token* cur = head;

    while(cur){
        printTokenKind(cur->kind); 
        printf("\n");
        cur = cur->next;
    }
}

/// Print a single TokenKind
void printTokenKind(TokenKind kind){
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