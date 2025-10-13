#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include <vector>
#include <map>
#include <string>

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
};

class Tokenizer {
public:
    class TokenStream {
        friend class Tokenizer;
        std::vector<Token> tokens;
        void clear() { tokens.clear(); }
        void addToken(const Token& token) { tokens.push_back(token); }
        void addToken(TokenKind kind, char* pos) {
            Token t;
            t.kind = kind;
            t.pos = pos;
            t.len = 1;
            t.val = 0;
            tokens.push_back(t);
        }
        Token& getTop() { return tokens.back(); }
    public:
        std::vector<Token>::iterator it;
        void reset() { it = tokens.begin(); }
        bool consume(TokenKind kind);
        void expect(TokenKind kind);
        int expectNum();
    };
    TokenStream& scan(char* p);
    Tokenizer();
    void printTokens();
private:
    std::map<std::string, TokenKind> keyword_map;
    TokenStream ts;
    TokenKind checkKeyword(char* start, unsigned int len);
    bool is_ident1(char c);
    bool is_ident2(char c);
    void printTokenKind(TokenKind kind);
};

Token* tokenize(char* p);
void print_tokens(Token* token);

typedef int ASTIdx;
enum class ASTKind {
    Num,
    Add,
    Sub,
    Mul,
    Div,
    Return,
};

struct ASTNode {
    ASTKind kind;
    ASTIdx lhs;
    ASTIdx rhs;

    union {
        struct Function {
            int a;
        } func;
        int val;
    } data;
};

class Parser{
public:
    Parser(Tokenizer::TokenStream& ts) : ts(ts) {}
    ASTIdx parseFile();
    void printAST(ASTIdx idx);
    ASTNode& getAST(ASTIdx idx) { return nodes[idx]; }
private:
    Tokenizer::TokenStream& ts;
    std::vector<ASTNode> nodes;
    ASTIdx stmt();
    ASTIdx expr();
    ASTIdx primary();
    ASTIdx newNode(ASTKind kind, ASTIdx lhs, ASTIdx rhs);
    ASTIdx newNodeNum(int val);

    int depth = 0;
    void printStmt(ASTIdx idx);
    void printExpr(ASTIdx idx);
    void printAST(const ASTNode& node, int depth = 0) const;
};

typedef unsigned int VReg;

enum class IRCmd {
    ADD,
    SUB,
    MUL,
    DIV,
    MOV,
    RET,
};


class IRInstr{
public:
    IRCmd cmd;
    VReg s1;
    VReg s2;
    VReg t;
};

class IRFunc{
public:
    std::string fname;
    std::vector<IRInstr> instrPool;
};

class IRModule{
public:
};

class IRGenerator{
public:
    Parser& ps;
    ASTIdx root;
    IRGenerator(ASTIdx idx, Parser parser) : ps(parser), root(idx) {}
    IRModule module;
    IRModule& run();
    void printIR(const IRModule& irm);
};
