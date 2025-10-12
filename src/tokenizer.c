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

/// Print the tokens in the linked list
void print_tokens(Token* head){
    Token* cur = head;

    while(cur){
        printf("kind: %d, val: %d\n", cur->kind, cur->val);
        cur = cur->next;
    }
}
