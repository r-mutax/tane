#include "tane.hpp"

void X86Generator::emit(){
    out.print(".intel_syntax noprefix\n");
    out.print(".text\n");

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

    for(auto i = 0; i < func.params.size(); i++){
        SymbolIdx symIdx = func.params[i];
        Symbol& sym = irm.getSymbol(symIdx);
        PhysReg paramReg;
        switch(i){
            case 0: paramReg = PhysReg::RDI; break;
            case 1: paramReg = PhysReg::RSI; break;
            case 2: paramReg = PhysReg::RDX; break;
            case 3: paramReg = PhysReg::RCX; break;
            case 4: paramReg = PhysReg::R8;  break;
            case 5: paramReg = PhysReg::R9;  break;
            default:
                fprintf(stderr, "More than 6 parameters not supported.\n");
                exit(1);
        }
        out.print("  mov [rbp - {}], {}\n", sym.stackOffset, regName(paramReg));
    }

    for(auto& instr : func.instrPool){
        func.regAlloc.expireAt(&instr - &func.instrPool[0]);
        switch(instr.cmd){
            case IRCmd::RET:
            {
                PhysReg r = func.regAlloc.alloc(instr.s1);
                if(r != PhysReg::RAX){
                    out.print("  mov rax, {}\n", regName(r));
                }
                out.print("  jmp ret_{}\n", func.fname);
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
            case IRCmd::FRAME_ADDR:
            {
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  lea {}, [rbp - {}]\n", regName(rt), instr.imm);
                break;
            }
            case IRCmd::LOAD:
            {
                PhysReg rAddr = func.regAlloc.alloc(instr.s1);
                PhysReg rt = func.regAlloc.alloc(instr.t);
                out.print("  mov {}, [{}]\n", regName(rt), regName(rAddr));
                break;
            }
            case IRCmd::SAVE:
            {
                PhysReg rAddr = func.regAlloc.alloc(instr.s1);
                PhysReg rVal = func.regAlloc.alloc(instr.s2);
                out.print("  mov [{}], {}\n", regName(rAddr), regName(rVal));
                break;
            }
            case IRCmd::LLABEL:
            {
                out.print(".L{}{}:\n", func.fname, instr.imm);
                break;
            }
            case IRCmd::JMP:
            {
                out.print("  jmp .L{}{}\n", func.fname, instr.imm);
                break;
            }
            case IRCmd::JZ:
            {
                PhysReg rCond = func.regAlloc.alloc(instr.s1);
                out.print("  cmp {}, 0\n", regName(rCond));
                out.print("  je .L{}{}\n", func.fname, instr.imm);
                break;
            }
            case IRCmd::CALL:
            {

                for (size_t i = 0; i < instr.args.size(); i++) {
                    PhysReg rArg = func.regAlloc.alloc(instr.args[i]);
                    static const char* argRegs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                    if (i < 6) {
                        out.print("  mov {}, {}\n", argRegs[i], regName(rArg));
                    } else {
                        // For simplicity, we won't handle more than 6 arguments here.
                        fprintf(stderr, "Error: More than 6 function arguments not supported in X86 generation.\n");
                        exit(1);
                    }
                }

                PhysReg r = func.regAlloc.alloc(instr.t);
                out.print("  call {}\n", irm.getSymbol(instr.imm).name);
                out.print("  mov {}, rax\n", regName(r));
                break;
            }
            case IRCmd::MOV:
            {
                PhysReg rSrc = func.regAlloc.alloc(instr.s1);
                PhysReg rDst = func.regAlloc.alloc(instr.t);
                out.print("  mov {}, {}\n", regName(rDst), regName(rSrc));
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
