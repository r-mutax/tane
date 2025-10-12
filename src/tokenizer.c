#include "tane.h"

Token* new_token(TokenKind kind, Token* cur, char* pos);

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
            default:
                if(isdigit(c)){
                    char* q = p;
                    int val = strtol(p, &p, 10);

                    cur = new_token(TK_NUM, cur, q);
                    cur->val = val;
                } else if(isspace(c)){
                    p++;
                } else {
                    fprintf(stderr, "Cannot tokenize: %s\n", p);
                    exit(1);
                }                
                break;
        }
    }

    return head.next;
}

Token* new_token(TokenKind kind, Token* cur, char* pos){
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->pos = pos;
    cur->next = tok;
    return tok;
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
        case TK_EOF:
            printf("TK_EOF");
            break;
        default:
            printf("Unknown TokenKind: %d", kind);
            break;
    }
}