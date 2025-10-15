#include "tane.hpp"

void X86Generator::emit(){
    out.print(".intel_syntax noprefix\n");

    for(auto& func : irm.funcPool){
        emitFunc(func);
    }
}

void X86Generator::emitFunc(IRFunc& func){

    out.print(".global {}\n", func.fname);
    out.print("{}:\n", func.fname);
    for(auto& instr : func.instrPool){
        switch(instr.cmd){
            case IRCmd::RET:
                out.print("  mov rax, {}\n", func.getVReg(instr.s1).val);
                out.print("  ret\n");
                break;
            default:
                fprintf(stderr, "Unknown IR command in X86 generation: %d\n", (unsigned int)instr.cmd);
                exit(1);
        }
    }
}
