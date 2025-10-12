#include "tane.h"

static Token* token;

static ASTNode* primary();
static bool consume_token(TokenKind kind);
static void expect_token(TokenKind kind);
static int expect_num();

static ASTNode* new_node(ASTNodeKind kind, ASTNode* lhs, ASTNode* rhs);
static ASTNode* new_node_num(int val);

ASTNode* parse(Token* tok) {
    // Implement parsing logic here
    token = tok;
    return primary();
}

static ASTNode* primary(){
    int n = expect_num();
    return new_node_num(n);
}

/// @brief Consume the current token if it matches the expected kind.
/// @param kind The kind of token to consume.
/// @return true if the token was consumed, false otherwise.
static bool consume_token(TokenKind kind){
    if(token->kind != kind) return false;
    token = token->next;
    return true;
}

/// @brief Expect the current token to be of a specific kind and consume it.
/// @param kind The expected kind of the token.
static void expect_token(TokenKind kind){
    if(token->kind != kind) {
        fprintf(stderr, "Unexpected token: %d\n", token->kind);
        exit(1);
    }
    token = token->next;
}

/// @brief Expect the current token to be a number and consume it.
/// @return The integer value of the number token.
static int expect_num(){
    if(token->kind != TK_NUM) {
        fprintf(stderr, "Expected a number but got: %d\n", token->kind);
        exit(1);
    }
    int val = token->val;
    token = token->next;
    return val;
}

/// @brief Create a new AST node.
/// @param kind The kind of the AST node.
/// @param lhs The left-hand side child.
/// @param rhs The right-hand-side child.
/// @return A pointer to the newly created AST node.
static ASTNode* new_node(ASTNodeKind kind, ASTNode* lhs, ASTNode* rhs) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static ASTNode* new_node_num(int val) {
    ASTNode* node = new_node(ND_NUM, NULL, NULL);
    node->val = val;
    return node;
}

// for debugging
void print_ast(ASTNode* node) {
    if(!node) return;

    printf("(");
    switch(node->kind) {
        case ND_NUM:
            printf("%d", node->val);
            break;
        default:
            break;
    }
    printf(")");
}