#include "tane.hpp"

void X86Generator::emit(){
    out.print(".intel_syntax noprefix\n");
    out.print(".global main\n");
    out.print("main:\n");
    out.print("  mov rax, 1\n");
    out.print("  ret\n");
}