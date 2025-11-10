.intel_syntax noprefix
.text

.global print
print:
    push rbp
    mov rbp, rsp
    mov rsi, rdi
    xor rdx, rdx
.L_strlen:
    cmp byte ptr [rdi], 0
    je .L_have_len
    inc rdx
    inc rdi
    jmp .L_strlen
.L_have_len:
    mov rax, 1  # sys_write
    mov rdi, 1  # fd = stdout
    syscall
    pop rbp
    ret

.global exit
exit:
    mov rax, 60  # sys_exit
    # rdi already contains exit status (first argument)
    syscall
