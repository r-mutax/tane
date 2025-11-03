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

    int32_t addString(std::string data){
        int32_t id = stringLiterals.size();
        StringLiteralData sld;
        sld.id = id;
        sld.str = data;
        stringLiterals.push_back(sld);
        return id;
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
            if(sym.isPub() == false){
                continue; // skip non-public symbols
            }
            if(sym.kind == SymbolKind::Function){
                fprintf(fp, "fn %s(", sym.name.c_str());
                for(size_t j = 0; j < sym.params.size(); j++){
                    const auto& paramSym = getSymbol(sym.params[j]);
                    fprintf(fp, "%s", paramSym.name.c_str());
                    if(j + 1 < sym.params.size()){
                        fprintf(fp, ", ");  
                    }
                }
                fprintf(fp, ");\n");
            }
        }
        fprintf(fp, "end\n");
        fclose(fp);
    }

    void printSymbols(){
        for(size_t i = 0; i < symbolPool.size(); i++){
            const auto& sym = symbolPool[i];
            std::cout << "Symbol[" << i << "]: " << sym.name << ", mut=" << sym.isMut() << "\n";
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
    TnlibLoader tnlibLoader;
        IRGenerator(ASTIdx idx, Parser& parser, ModulePath& mPath) : ps(parser), root(idx), tnlibLoader(mPath) {
        module.scopes.push_back(Scope(-1));
        module.curScope = module.scopes.size() - 1;
        module.globalScope = module.scopes.size() - 1;
    }
    IRModule module;
    IRModule& run();
    void printIR(const IRModule& irm);

};
