.intel_syntax   noprefix
.global         main, plus


plus:
    add     rsi, rdi
    mov     rax, rsi
    ret

main:
    mov     rdi, 2
    mov     rsi, 3
    call    plus
    ret
