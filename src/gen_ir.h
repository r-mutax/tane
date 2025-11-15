#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "common_type.h"
#include "symbol.h"
#include "parse.h"
#include "tnlib_loader.h"

enum class PhysReg : uint8_t { None, R10, R11, R12, R13, R14, R15, RAX, RDI, RSI, RDX, RCX, R8, R9 };
enum class VRegKind : uint8_t { Temp, Imm, LVarAddr };

class VReg{
public:
    VRegKind kind = VRegKind::Temp;
    int32_t val;
    PhysReg assigned = PhysReg::None;
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
    LEA_STRING,
};

class IRInstr{
public:
    IRCmd cmd;
    VRegID s1 = -1;
    VRegID s2 = -1;
    VRegID t = -1;
    int32_t imm = 0;
    std::vector<VRegID> args;
};

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
        void computeUse();

        void expireAt(size_t pos);

        PhysReg alloc(VRegID vid);
    };

    RegAlloc regAlloc{*this};
public:
    uint32_t localStackSize = 0;
    void clean();
    std::string fname;
    void newIRInstr(const IRCmd cmd, VRegID s1 = -1, VRegID s2 = -1, VRegID t = -1);

    int32_t newLabel();
    VRegID newVReg();
    VRegID newVRegNum(int32_t val);
    VRegID newVRegVar(Symbol sym);
    VReg& getVReg(VRegID id);
};

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

// Function semantics information
struct FuncSem{
    uint32_t localBytes{0};             // local variable stack size in bytes
    std::vector<SymbolIdx> params;      // function parameters
};

struct StringLiteralData{
    int32_t id;
    std::string str;
};

class IRModule{
public:
    std::vector<IRFunc> funcPool;
    std::vector<Scope> scopes;
    std::vector<Symbol> symbolPool;
    std::vector<StringLiteralData> stringLiterals;

    std::unordered_map<ASTIdx, FuncSem> funcSem;
    std::unordered_map<ASTIdx, SymbolIdx> astSymMap;

    ScopeIdx curScope = -1;
    ScopeIdx globalScope = -1;
    ScopeIdx funcScope = -1;
    uint32_t currentStackSize = 0;

    SymbolIdx findSymbol(const std::string& name, ScopeIdx idx);

    void scopeIn();

    void scopeOut();

    int32_t addString(std::string data);

    SymbolIdx insertSymbol(const Symbol& sym);

    Symbol& getSymbol(SymbolIdx idx);

    void outputSymbols(std::string dir, std::string module);

    void printSymbols();
};

class IRGenerator{
private:
    IRFunc* curFunc;
    void bindTU(ASTIdx idx);
    void bindImport(ASTIdx idx);
    void bindFunc(ASTIdx idx);
    void bindStmt(ASTIdx idx);
    void bindExpr(ASTIdx idx);
    IRFunc* genFunc(ASTIdx idx);
    void genStmt(ASTIdx idx);
    VRegID genExpr(ASTIdx idx);
    VRegID genlvalue(ASTIdx idx);

public:
    Parser& ps;
    ASTIdx root;
    TnlibLoader tnlibLoader;
    IRGenerator(ASTIdx idx, Parser& parser, ModulePath& mPath, moduleSet& loadedModules) : ps(parser), root(idx), tnlibLoader(mPath, loadedModules) {
        module.scopes.push_back(Scope(-1));
        module.curScope = module.scopes.size() - 1;
        module.globalScope = module.scopes.size() - 1;
    }
    IRModule module;
    IRModule& run();
    void printIR(const IRModule& irm);

};
