#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <format>
#include <string_view>
#include <cstdint>

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
    int32_t val;
    char* pos;
    int32_t len;
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
        int32_t expectNum();
    };
    TokenStream& scan(char* p);
    Tokenizer();
    void printTokens();
private:
    std::map<std::string, TokenKind> keyword_map;
    TokenStream ts;
    TokenKind checkKeyword(char* start, uint32_t len);
    bool is_ident1(char c);
    bool is_ident2(char c);
    void printTokenKind(TokenKind kind);
};

Token* tokenize(char* p);
void print_tokens(Token* token);

typedef int32_t ASTIdx;
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
            int32_t a;
        } func;
        int32_t val;
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
    ASTIdx add();
    ASTIdx primary();
    ASTIdx newNode(ASTKind kind, ASTIdx lhs, ASTIdx rhs);
    ASTIdx newNodeNum(int32_t val);

    int32_t depth = 0;
    void printStmt(ASTIdx idx);
    void printExpr(ASTIdx idx);
    void printAST(const ASTNode& node, int32_t depth = 0) const;
};

typedef int32_t VRegID;
class VReg{
public:
    int32_t val;
};

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
    VRegID s1 = -1;
    VRegID s2 = -1;
    VRegID t = -1;
};

enum class PhysReg : uint8_t { None, R10, R11, R12, R13, R14, R15, RAX };

inline const char* regName(PhysReg r) {
    switch(r) {
        case PhysReg::R10: return "r10";
        case PhysReg::R11: return "r11";
        case PhysReg::R12: return "r12";
        case PhysReg::R13: return "r13";
        case PhysReg::R14: return "r14";
        case PhysReg::R15: return "r15";
        case PhysReg::RAX: return "rax";
        default: return "none";
    }
}

class IRFunc{
    friend class IRGenerator;
    friend class X86Generator;
    std::vector<IRInstr> instrPool;
    std::vector<VReg> vregs;
public:
    void clean(){
        instrPool.clear();
        fname = "";
    }
    std::string fname;
    VRegID newVRegNum(int32_t val){
        VReg vr;
        vr.val = val;
        vregs.push_back(vr);
        return vregs.size() - 1;
    }
    VReg& getVReg(VRegID id){
        if(id < 0){
            fprintf(stderr, "VReg is not used.\n");
            exit(1);
        }
        if((size_t)id >= vregs.size()){
            fprintf(stderr, "Invalid VRegID: %d\n", id);
            exit(1);
        }
        return vregs[id];
    }
};

class IRModule{
public:
    std::vector<IRFunc> funcPool;
};

class IRGenerator{
private:
    IRFunc func;
    IRFunc genFunc(void);
    void genStmt(ASTIdx idx);
    VRegID genExpr(ASTIdx idx);
public:
    Parser& ps;
    ASTIdx root;
    IRGenerator(ASTIdx idx, Parser& parser) : ps(parser), root(idx) {}
    IRModule module;
    IRModule& run();
    void printIR(const IRModule& irm);
};

/// Output context interface
class OutputContext{
public:
    virtual ~OutputContext() {}
    virtual void write(std::string_view str) = 0;
    virtual void flush() = 0;
};

/// Standard output context
class StdioContext : public OutputContext{
public:
    void write(std::string_view str) override {
        std::cout << str;
    }
    void flush() override {
        std::cout.flush();
    }
};

/// @brief File output context
class FileContext : public OutputContext{
    FILE* fp;
public:
    FileContext(const char* filename) : fp(nullptr) {
        fp = fopen(filename, "w");
        if(!fp) {
            fprintf(stderr, "Cannot open file: %s\n", filename);
            exit(1);
        }
    }
    void write(std::string_view str) override {
        if(fp) {
            fputs(str.data(), fp);
        }
    }
    void flush() override {
        if(fp) {
            fflush(fp);
        }
    }
};

class NullContext : public OutputContext{
public:
    void write(std::string_view) override {}
    void flush() override {}
};

class Output {
    OutputContext* ctx;
public:
    Output(OutputContext* ctx_) : ctx(ctx_) {}
    Output() : ctx(new NullContext()) {}
    template <class... Ts>
    void print(std::format_string<Ts...> fmt, Ts&&... args) {
        std::string buf;
        buf.reserve(128);
        std::format_to(std::back_inserter(buf), fmt, std::forward<Ts>(args)...);
        ctx->write(buf);
    }
    void flush() {
        ctx->flush();
    }
    void setFileContext(const char* filename) {
        delete ctx;
        ctx = new FileContext(filename);
    }
    void setStdioContext() {
        delete ctx;
        ctx = new StdioContext();
    }
};

class X86Generator{
    IRModule& irm;
    Output out;
    void emitFunc(IRFunc& func);
public:
    X86Generator(IRModule& irm_) : irm(irm_), out() {}
    void setOutputFile(const char* filename) {
        out.setFileContext(filename);
    }
    void emit();
};
