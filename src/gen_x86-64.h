#pragma once
#include "gen_ir.h"
#include "context.h"

class X86Generator{
    IRModule& irm;
    Output out;
    void emitFunc(IRFunc& func);
    void emitStringLiterals();
public:
    X86Generator(IRModule& irm_) : irm(irm_), out() {}
    void setOutputFile(const char* filename) {
        out.setFileContext(filename);
    }
    void emit();
};
