#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct Token Token;
typedef struct ASTNode ASTNode;

typedef enum TokenKind {
    TK_NUM,
    TK_ADD,         // +
    TK_SUB,         // -
    TK_MUL,         // *
    TK_DIV,         // /
    TK_EOF,
} TokenKind;

struct Token{
    TokenKind kind;
    int val;
    char* pos;
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
} ASTNodeKind;

struct ASTNode {
    ASTNodeKind kind;
    ASTNode* lhs;
    ASTNode* rhs;
    int val;        // used if kind == ND_NUM
};

ASTNode* parse(Token* token);
