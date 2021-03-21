#if defined(__x86_64__)

    .data
saved_rip:
    .long 0

    .text

    .global posix_sigsegv_caller_save_rip
    .type posix_sigsegv_caller_save_rip, @function
posix_sigsegv_caller_save_rip:
    leaq saved_rip(%rip), %rax
    movq %rdi, (%rax)
    ret

    .global posix_sigsegv_caller
    .type posix_sigsegv_caller, @function
posix_sigsegv_caller:
    movq saved_rip(%rip), %rax
    push %rax

    pushq %rbp
    movq %rsp, %rbp
    movq %rax, %rdi
    call posix_sigsegv_callee
    movl $1, %edi
    call exit

#endif /* __x86_64__ */
