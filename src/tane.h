#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct Token Token;
typedef struct ASTNode ASTNode;
typedef struct IR IR;
typedef struct Operand Operand;

typedef enum TokenKind {
    TK_NUM,
    TK_ADD,         // +
    TK_SUB,         // -
    TK_MUL,         // *
    TK_DIV,         // /
    TK_L_PAREN,     // (
    TK_R_PAREN,     // )
    TK_SEMICOLON,   // ;
    TK_RETURN,      // "return"
    TK_IDENT,       // Identifier
    TK_EOF,
} TokenKind;

struct Token{
    TokenKind kind;
    int val;
    char* pos;
    int len;
    Token* next;
};
Token* tokenize(char* p);
void print_tokens(Token* token);

typedef enum ASTNodeKind {
    ND_NUM,
    ND_ADD,         // +
    ND_SUB,         // -
    ND_MUL,         // *
    ND_DIV,         // /
    ND_RETURN,      // "return"
} ASTNodeKind;

struct ASTNode {
    ASTNodeKind kind;
    ASTNode* lhs;
    ASTNode* rhs;

    union {
        struct function {
            int a;
        } func;
        int val;
    } data;
};

ASTNode* parse(Token* token);
void print_ast(ASTNode* node);

typedef enum IRCmd {
    IR_FNAME,
    IR_FHEAD,
    IR_FTAIL,
    IR_ADD,
    IR_RETURN,
} IRCmd;

struct IR {
    IRCmd cmd;
    Operand* target;
    Operand* lhs;
    Operand* rhs;
    IR* next;
};

typedef enum OperandKind{
    OP_REG,
    OP_IMM,
} OperandKind;

struct Operand {
    OperandKind kind;

    int val;
};

IR* gen_ir(ASTNode* node);
void print_ir(IR* ir);


typedef struct KEYWORD_MAP {
    char*       keyword;
    TokenKind   kind;
} KEYWORD_MAP;

static KEYWORD_MAP keyword_map[] = {
    {"return",        TK_RETURN},
};

