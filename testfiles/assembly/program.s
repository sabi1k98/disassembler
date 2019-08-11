.type npown, @function
.GLOBAL npown
npown:
    mov %rsi, %rax
    mov %rdi, %rbx
    cmp $0, %rax
    je one
    jmp loop
mul:
    imul %rbx, %rdi
loop:
    add $-1, %rax
    cmp $0, %rax
    jne mul
    jmp end
one:
    add $1, %rax
    ret
end:
    mov %rdi, %rax
    ret
