.type main, @function
.type foo, @function
.type boo, @function
.type break, @function
.GLOBAL foo
.GLOBAL boo
.GLOBAL break
.GLOBAL main

main:
    jne foo
    jne boo
    jne break
    ret

foo:
    mov %rax, %rbx
    nop
    int3
    cmp $0, %rax
    jne .random_label
.random_label:
    add $420000, %rax
    ret

boo:
    jmp .reachable_label
.unreachable_label: # shouldnt be decoded using recursive traversal
    nop
    nop
    nop
.reachable_label:
    ret

break:
    jmp 26 # jumps into the middle of mul
    imul %rax, %rdx
    ret
