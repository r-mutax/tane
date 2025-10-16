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
    TK_MOD,         // %
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
    Mod,
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
    ASTIdx mul();
    ASTIdx primary();
    ASTIdx newNode(ASTKind kind, ASTIdx lhs, ASTIdx rhs);
    ASTIdx newNodeNum(int32_t val);

    int32_t depth = 0;
    void printStmt(ASTIdx idx);
    void printExpr(ASTIdx idx);
    void printAST(const ASTNode& node, int32_t depth = 0) const;
};

enum class PhysReg : uint8_t { None, R10, R11, R12, R13, R14, R15, RAX };

typedef int32_t VRegID;
class VReg{
public:
    int32_t val;
    PhysReg assigned = PhysReg::None;
};


enum class IRCmd {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    MOV,
    MOV_IMM,
    RET,
};


class IRInstr{
public:
    IRCmd cmd;
    VRegID s1 = -1;
    VRegID s2 = -1;
    VRegID t = -1;
    int32_t imm = 0;
};


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
    friend class RegAlloc;
    std::vector<IRInstr> instrPool;
    std::vector<VReg> vregs;

    class RegAlloc{
        std::vector<PhysReg> freeRegs;
        std::vector<VRegID> lastUse;
        IRFunc& f;
    public:
        RegAlloc(IRFunc& func) : f(func) {
            freeRegs = {PhysReg::R10, PhysReg::R11, PhysReg::R12, PhysReg::R13, PhysReg::R14, PhysReg::R15};
        }
        void computeUse(){
            size_t nV = f.vregs.size();
            lastUse.assign(nV, -1);
            for(size_t i = 0; i < f.instrPool.size(); i++) {
                const auto& ins = f.instrPool[i];
                auto mark = [&](VRegID v) {
                    if(v >= 0 && (size_t)v < nV) {
                        lastUse[v] = std::max(lastUse[v], (VRegID)i);
                    }
                };
                mark(ins.s1);
                mark(ins.s2);
                mark(ins.t);
            }
        }
        void expireAt(size_t pos){
            for(size_t vid = 0; vid < f.vregs.size(); vid++){
                auto& vr = f.vregs[vid];
                if(vr.assigned != PhysReg::None     // assigned to physical register_
                    && lastUse[vid] >= 0            // used at least once
                    && lastUse[vid] < (VRegID)pos)          // last used before current position  
                {
                    freeRegs.push_back(vr.assigned);
                    vr.assigned = PhysReg::None;
                }
            }
        }
        PhysReg alloc(VRegID vid){
            auto& vr = f.getVReg(vid);
            if(vr.assigned != PhysReg::None) return vr.assigned;
            if(!freeRegs.empty()){
                PhysReg r = freeRegs.back();
                freeRegs.pop_back();
                vr.assigned = r;
                return r;
            }
            fprintf(stderr, "No free registers available for VReg %d\n", vid);
            exit(1);
        }
    };

    RegAlloc regAlloc{*this};
public:
    void clean(){
        instrPool.clear();
        fname = "";
    }
    std::string fname;
    void newIRInstr(const IRCmd cmd, VRegID s1 = -1, VRegID s2 = -1, VRegID t = -1) {
        IRInstr instr;
        instr.cmd = cmd;
        instr.s1 = s1;
        instr.s2 = s2;
        instr.t = t;
        instrPool.push_back(instr);
    }
    VRegID newVReg(){
        VReg vr;
        vregs.push_back(vr);
        return vregs.size() - 1;
    }
    VRegID newVRegNum(int32_t val){
        VRegID vid = newVReg();
        IRInstr isntr;
        isntr.cmd = IRCmd::MOV_IMM;
        isntr.t = vid;
        isntr.imm = val;
        instrPool.push_back(isntr);
        return vid;
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
