#include "tane.hpp"

void X86Generator::emit(){
    out.print(".intel_syntax noprefix\n");

    for(auto& func : irm.funcPool){
        emitFunc(func);
    }
}

void X86Generator::emitFunc(IRFunc& func){
    func.regAlloc.computeUse();
    
    out.print(".global {}\n", func.fname);
    out.print("{}:\n", func.fname);
    for(auto& instr : func.instrPool){
        func.regAlloc.expireAt(&instr - &func.instrPool[0]);
        switch(instr.cmd){
            case IRCmd::RET:
            {
                PhysReg r = func.regAlloc.alloc(instr.s1);
                if(r != PhysReg::RAX){
                    out.print("  mov rax, {}\n", regName(r));
                }
                out.print("  ret\n");
                break;
            }
            case IRCmd::MOV_IMM:
            {
                PhysReg r = func.regAlloc.alloc(instr.t);
                out.print("  mov {}, {}\n", regName(r), instr.imm);
                break;
            }
            default:
                fprintf(stderr, "Unknown IR command in X86 generation: %d\n", (uint32_t)instr.cmd);
                exit(1);
        }
    }
}
