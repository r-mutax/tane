#include "tane.hpp"

void X86Generator::emit(){
    out.print(".intel_syntax noprefix\n");
    out.print(".text:\n");

    for(auto& func : irm.funcPool){
        emitFunc(func);
    }
}

void X86Generator::emitFunc(IRFunc& func){
    func.regAlloc.computeUse();
    
    out.print(".global {}\n", func.fname);
    out.print("{}:\n", func.fname);
    out.print("  push rbp\n");
    out.print("  mov rbp, rsp\n");
    out.print("  sub rsp, {}\n", func.localStackSize);
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
            case IRCmd::ADD:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  add {}, {}\n", regName(rt), regName(r2));
                break;
            }
            case IRCmd::SUB:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  sub {}, {}\n", regName(rt), regName(r2));
                break;
            }
            case IRCmd::MUL:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  imul {}, {}\n", regName(rt), regName(r2));
                break;
            }
            case IRCmd::DIV:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(r1 != PhysReg::RAX){
                    out.print("  mov rax, {}\n", regName(r1));
                }
                out.print("  cqo\n");
                out.print("  idiv {}\n", regName(r2));
                if(rt != PhysReg::RAX){
                    out.print("  mov {}, rax\n", regName(rt));
                }
                break;
            }
            case IRCmd::MOD:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(r1 != PhysReg::RAX){
                    out.print("  mov rax, {}\n", regName(r1));
                }
                out.print("  cqo\n");
                out.print("  idiv {}\n", regName(r2));
                out.print("  mov {}, rdx\n", regName(rt));
                break;
            }
            case IRCmd::LOGICAL_OR:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  cmp {}, 0\n", regName(r1));
                out.print("  setne al\n");
                out.print("  cmp {}, 0\n", regName(r2));
                out.print("  setne cl\n");
                out.print("  or al, cl\n");
                out.print("  movzx {}, al\n", regName(rt));
                break;
            }
            case IRCmd::LOGICAL_AND:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  cmp {}, 0\n", regName(r1));
                out.print("  setne al\n");
                out.print("  cmp {}, 0\n", regName(r2));
                out.print("  setne cl\n");
                out.print("  and al, cl\n");
                out.print("  movzx {}, al\n", regName(rt));
                break;
            }
            case IRCmd::BIT_OR:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  or {}, {}\n", regName(rt), regName(r2));
                break;
            }
            case IRCmd::BIT_XOR:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  xor {}, {}\n", regName(rt), regName(r2));
                break;
            }
            case IRCmd::BIT_AND:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  and {}, {}\n", regName(rt), regName(r2));
                break;
            }
            case IRCmd::EQUAL:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  cmp {}, {}\n", regName(r1), regName(r2));
                out.print("  sete al\n");
                out.print("  movzx {}, al\n", regName(rt));
                break;
            }
            case IRCmd::NEQUAL:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  cmp {}, {}\n", regName(r1), regName(r2));
                out.print("  setne al\n");
                out.print("  movzx {}, al\n", regName(rt));
                break;
            }
            case IRCmd::LT:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  cmp {}, {}\n", regName(r1), regName(r2));
                out.print("  setl al\n");
                out.print("  movzx {}, al\n", regName(rt));
                break;
            }
            case IRCmd::LE:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  cmp {}, {}\n", regName(r1), regName(r2));
                out.print("  setle al\n");
                out.print("  movzx {}, al\n", regName(rt));
                break;
            }
            case IRCmd::LSHIFT:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  mov cl, {}\n", regName8(r2));
                out.print("  shl {}, cl\n", regName(rt));
                break;
            }
            case IRCmd::RSHIFT:
            {
                PhysReg r1 = func.regAlloc.alloc(instr.s1);
                PhysReg r2 = func.regAlloc.alloc(instr.s2);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                if(rt != r1){
                    out.print("  mov {}, {}\n", regName(rt), regName(r1));
                }
                out.print("  mov cl, {}\n", regName8(r2));
                out.print("  shr {}, cl\n", regName(rt));
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

    out.print("ret_{}:\n", func.fname);
    out.print("  mov rsp, rbp\n");
    out.print("  pop rbp\n");
    out.print("  ret\n");
}
