section .text
bits 64
default rel

global daxpy_asm

; ecx = n
; xmm1 = a
; r8 = x
; r9 = y
; z is the fifth parameter

daxpy_asm:
    push rbp
    mov rbp, rsp
    add rbp, 16

    mov r10, [rbp + 32]
    cmp ecx, 0
    jle done

daxpy_loop:
    movsd xmm0, [r8]
    mulsd xmm0, xmm1
    addsd xmm0, [r9]
    movsd [r10], xmm0

    add r8, 8
    add r9, 8
    add r10, 8
    dec ecx
    jne daxpy_loop

done:
    pop rbp
    ret
