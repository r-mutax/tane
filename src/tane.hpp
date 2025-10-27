#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <format>
#include <string_view>
#include <cstdint>
#include <optional>

typedef int32_t SymbolIdx;

enum class TokenKind {
    Num,
    Add,         // +
    Sub,         // -
    Mul,         // *
    Div,         // /
    Mod,         // %
    Equal,       // =
    EqualEqual,  // ==
    NotEqual,    // !=
    LessThan,    // <
    LessEqual,   // <=
    LShift,      // <<
    RShift,      // >>
    And,         // &
    Hat,         // ^
    Or,          // |
    AndAnd,      // &&
    OrOr,        // ||
    LParen,      // (
    RParen,      // )
    LBrace,      // {
    RBrace,      // }
    Semicolon,   // ;
    Comma,       // ,
    EqualArrow,  // =>
    Return,      // "return"
    Let,         // "let"
    Mut,         // "mut"
    If,          // "if"
    Else,        // "else"
    While,       // "while"
    Switch,      // "switch"
    Fn,          // "fn"
    Import,      // "import"
    Ident,       // Identifier
    // =========== for tnlib ===========
    Tnlib,
    Module,
    Eof,
};

struct Token{
    TokenKind kind;
    int32_t val;
    char* pos;
    int32_t len;
};

typedef uint32_t TokenIdx;

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
        TokenIdx idx;
        Token getToken(TokenIdx idx) { return tokens[idx]; }
        void reset() { idx = 0; }
        bool consume(TokenKind kind);
        void expect(TokenKind kind);
        int32_t expectNum();
        TokenIdx expectIdent();
        std::optional<TokenIdx> consumeIdent();
        bool peekKind(TokenKind kind, TokenIdx offset = 0){
            size_t i = static_cast<size_t>(idx) + static_cast<size_t>(offset);
            if(i >= tokens.size()) return false;
            return tokens[i].kind == kind;
        }
    };
    TokenStream& scan(char* p);
    Tokenizer(bool for_tnlib = false) : for_tnlib(for_tnlib) {}
    bool for_tnlib;
    void printTokens(TokenStream& ts);
private:
    static std::map<std::string, TokenKind> keyword_map;
    static std::map<std::string, TokenKind> tnlib_keyword_map;
    //TokenStream ts;
    TokenKind checkKeyword(char* start, uint32_t len);
    TokenKind checkKeywordTnlib(char* start, uint32_t len);
    bool is_ident1(char c);
    bool is_ident2(char c);
    void printTokenKind(TokenKind kind);
};

Token* tokenize(char* p);
void print_tokens(Token* token);

class ModulePath{
    std::vector<std::string> tnlibDirs;
public:
    ModulePath() = default;
    void addDirPath(const std::string& path){
        tnlibDirs.push_back(path);
    }
    std::string resolve(const std::string& moduleName){
        for(const auto& dir : tnlibDirs){
            std::string fullPath = dir + "/" + moduleName + ".tnlib";
            FILE* f = fopen(fullPath.c_str(), "r");
            if(f != nullptr){
                fclose(f);
                return fullPath;
            }
        }
        return "";
    }
};

typedef int32_t ASTIdx;
enum class ASTKind {
    TranslationUnit,
    Function,
    Num,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    LogicalOr,
    LogicalAnd,
    BitOr,
    BitXor,
    BitAnd,
    Equal,
    NotEqual,
    LessThan,
    LessEqual,
    LShift,
    RShift,
    CompoundStmt,
    Return,
    VarDecl,
    Variable,
    Assign,
    If,
    While,
    Switch,
    Case,
    FunctionCall,
    Import,
};

struct ASTNode {
    ASTKind kind;
    ASTIdx lhs;
    ASTIdx rhs;

    // For Num
    int32_t val;

    // For CompoundStmt or Function
    std::vector<ASTIdx> body;

    // For Function
    std::vector<ASTIdx> params;

    // For FunctionCall
    std::vector<ASTIdx> args;
    
    // For VarDecl
    std::string name;
    bool is_mut;

    // For if statement
    ASTIdx cond;
    ASTIdx thenBr;
    ASTIdx elseBr;

    SymbolIdx symIdx;
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
    ASTIdx functionDef();
    ASTIdx compoundStmt();
    ASTIdx stmt();
    ASTIdx expr();
    ASTIdx logical_or();
    ASTIdx logical_and();
    ASTIdx bitwise_or();
    ASTIdx bitwise_xor();
    ASTIdx bitwise_and();
    ASTIdx equality();
    ASTIdx relational();
    ASTIdx shift();
    ASTIdx add();
    ASTIdx mul();
    ASTIdx unary();
    ASTIdx primary();
    ASTIdx newNode(ASTKind kind, ASTIdx lhs, ASTIdx rhs);
    ASTIdx newNodeNum(int32_t val);

    int32_t depth = 0;
    void printStmt(ASTIdx idx);
    void printExpr(ASTIdx idx);
    void printAST(const ASTNode& node, int32_t depth = 0) const;
};

enum class PhysReg : uint8_t { None, R10, R11, R12, R13, R14, R15, RAX, RDI, RSI, RDX, RCX, R8, R9 };
enum class VRegKind : uint8_t { Temp, Imm, LVarAddr };
typedef int32_t VRegID;
class VReg{
public:
    VRegKind kind = VRegKind::Temp;
    int32_t val;
    PhysReg assigned = PhysReg::None;
};

enum class SymbolKind : uint8_t { Variable, Function };

class Symbol{
public:
    SymbolKind kind = SymbolKind::Variable;
    std::string name;
    TokenIdx tokenIdx;
    bool isMut;
    uint32_t stackOffset = 0;
    std::vector<SymbolIdx> params;
};


enum class IRCmd {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LOGICAL_OR,
    LOGICAL_AND,
    BIT_XOR,
    BIT_OR,
    BIT_AND,
    EQUAL,
    NEQUAL,
    LT,
    LE,
    LSHIFT,
    RSHIFT,
    MOV,
    MOV_IMM,
    RET,
    LOAD,
    SAVE,
    FRAME_ADDR,
    LLABEL,
    JZ,             // jmp if zero
    JNZ,
    JMP,            // unconditional jmp
    CALL,
};
/*
    cond
    jz cond .Llse
    then
    jmp .Lend
    .Lelse
    else
    .Lend
*/

class IRInstr{
public:
    IRCmd cmd;
    VRegID s1 = -1;
    VRegID s2 = -1;
    VRegID t = -1;
    int32_t imm = 0;
    std::vector<VRegID> args;
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
        case PhysReg::RDI: return "rdi";
        case PhysReg::RSI: return "rsi";
        case PhysReg::RDX: return "rdx";
        case PhysReg::RCX: return "rcx";
        case PhysReg::R8:  return "r8";
        case PhysReg::R9:  return "r9";
        default: return "none";
    }
}

inline const char* regName8(PhysReg r) {
    switch(r) {
        case PhysReg::R10: return "r10b";
        case PhysReg::R11: return "r11b";
        case PhysReg::R12: return "r12b";
        case PhysReg::R13: return "r13b";
        case PhysReg::R14: return "r14b";
        case PhysReg::R15: return "r15b";
        case PhysReg::RAX: return "al";
        case PhysReg::RDI: return "dil";
        case PhysReg::RSI: return "sil";
        case PhysReg::RDX: return "dl";
        case PhysReg::RCX: return "cl";
        case PhysReg::R8:  return "r8b";
        case PhysReg::R9:  return "r9b";
        default: return "none";
    }
}

class IRFunc{
    friend class IRGenerator;
    friend class X86Generator;
    friend class RegAlloc;
    std::vector<IRInstr> instrPool;
    std::vector<VReg> vregs;
    std::vector<SymbolIdx> params;
    int32_t labelCounter = 0;

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
                for(auto arg : ins.args){
                    mark(arg);
                }
            }
        }
        void expireAt(size_t pos){
            for(size_t vid = 0; vid < f.vregs.size(); vid++){
                auto& vr = f.vregs[vid];
                if(vr.assigned != PhysReg::None     // assigned to physical register
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
    uint32_t localStackSize = 0;
    void clean(){
        instrPool.clear();
        fname = "";
        labelCounter = 0;
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


    int32_t newLabel(){
        return labelCounter++;
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
    VRegID newVRegVar(Symbol sym){
        
        VRegID addrVid = newVReg();
        IRInstr addrInstr;
        addrInstr.cmd = IRCmd::FRAME_ADDR;
        addrInstr.t = addrVid;
        addrInstr.imm = sym.stackOffset;
        instrPool.push_back(addrInstr);

        VRegID vid = newVReg();
        IRInstr instr;
        instr.cmd = IRCmd::LOAD;
        instr.t = vid;
        instr.s1 = addrVid;
        instrPool.push_back(instr);
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

typedef int32_t ScopeIdx;
class Scope{
public:
    std::unordered_map<std::string, SymbolIdx> symbols;
    ScopeIdx parent = -1;

    Scope(ScopeIdx p) : parent(p) {}
    void insertSymbol(const std::string& name, SymbolIdx idx){
        symbols[name] = idx;
    }
    SymbolIdx findSymbol(const std::string& name){
        auto it = symbols.find(name);
        if(it != symbols.end()){
            return it->second;
        } else {
            return -1;  // not found
        }
    }
};

struct FuncSem{
    uint32_t localBytes{0};
    std::vector<SymbolIdx> params;
};

class IRModule{
public:
    std::vector<IRFunc> funcPool;
    std::vector<Scope> scopes;
    std::vector<Symbol> symbolPool;

    std::unordered_map<ASTIdx, FuncSem> funcSem;
    std::unordered_map<ASTIdx, SymbolIdx> astSymMap;

    ScopeIdx curScope = -1;
    ScopeIdx globalScope = -1;
    ScopeIdx funcScope = -1;
    uint32_t currentStackSize = 0;

    SymbolIdx findSymbol(const std::string& name, ScopeIdx idx){
        if(idx < 0 || (size_t)idx >= scopes.size()){
            return -1; // not found
        }
        Scope& sc = scopes[idx];

        SymbolIdx symIdx = sc.findSymbol(name);
        if(symIdx != -1){
            return symIdx;
        }

        if(sc.parent != -1){
            return findSymbol(name, sc.parent);
        }

        return -1; // not found
    }

    void scopeIn(){
        ScopeIdx parentIdx = curScope;
        scopes.push_back(Scope(parentIdx));
        curScope = scopes.size() - 1;

        // when into function scope
        if(parentIdx == globalScope){
            funcScope = curScope;
        }
    };

    void scopeOut(){
        Scope& sc = scopes[curScope];
        if(sc.parent == -1){
            fprintf(stderr, "Cannot scope out from global scope.\n");
            exit(1);
        }

        curScope = sc.parent;

        // when out of function scope
        if(curScope == globalScope){
            funcScope = -1;
        }
    }

    SymbolIdx insertSymbol(const Symbol& sym){
        // check duplication
        Scope& sc = scopes[curScope];
        if(sc.symbols.find(sym.name) != sc.symbols.end()){
            fprintf(stderr, "Symbol already exists in current scope: %s\n", sym.name.c_str());
            exit(1);
        }

        // add to pool
        symbolPool.push_back(sym);
        SymbolIdx symIdx = symbolPool.size() - 1;

        // add to scope
        sc.insertSymbol(sym.name, symIdx);

        Symbol& s = symbolPool[symIdx];
        currentStackSize += 8; // assuming 8 bytes per variable
        s.stackOffset = currentStackSize;

        return symIdx;
    }

    Symbol& getSymbol(SymbolIdx idx){
        if(idx < 0 || (size_t)idx >= symbolPool.size()){
            fprintf(stderr, "Invalid SymbolIdx: %d\n", idx);
            exit(1);
        }
        return symbolPool[idx];
    }

    void outputSymbols(std::string module){
        FILE *fp;
        std::string filename = module + ".tnlib";
        fp = fopen(filename.c_str(), "w");
        if(!fp){
            fprintf(stderr, "Cannot open file: %s\n", filename.c_str());
            exit(1);
        }

        fprintf(fp, "tnlib 1\n");
        fprintf(fp, "module %s\n", module.c_str());
        for(size_t i = 0; i < symbolPool.size(); i++){
            const auto& sym = symbolPool[i];
            if(sym.kind == SymbolKind::Function){
                fprintf(fp, "fn %s(", sym.name.c_str());
                for(size_t j = 0; j < sym.params.size(); j++){
                    const auto& paramSym = getSymbol(sym.params[j]);
                    fprintf(fp, "%s", paramSym.name.c_str());
                    if(j + 1 < sym.params.size()){
                        fprintf(fp, ", ");  
                    }
                }
                fprintf(fp, ")\n");
            }
        }
        fprintf(fp, "end\n");
        fclose(fp);
    }

    void printSymbols(){
        for(size_t i = 0; i < symbolPool.size(); i++){
            const auto& sym = symbolPool[i];
            std::cout << "Symbol[" << i << "]: " << sym.name << ", mut=" << sym.isMut << "\n";
        }
    }

};

class IRGenerator{
private:
    IRFunc func;
    void bindTU(ASTIdx idx);
    void bindImport(ASTIdx idx);
    void bindFunc(ASTIdx idx);
    void bindStmt(ASTIdx idx);
    void bindExpr(ASTIdx idx);
    IRFunc genFunc(ASTIdx idx);
    void genStmt(ASTIdx idx);
    VRegID genExpr(ASTIdx idx);
    VRegID genlvalue(ASTIdx idx);

public:
    Parser& ps;
    ASTIdx root;
    ModulePath& modulePath;
    IRGenerator(ASTIdx idx, Parser& parser, ModulePath& mPath) : ps(parser), root(idx), modulePath(mPath) {
        module.scopes.push_back(Scope(-1));
        module.curScope = module.scopes.size() - 1;
        module.globalScope = module.scopes.size() - 1;
    }
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
