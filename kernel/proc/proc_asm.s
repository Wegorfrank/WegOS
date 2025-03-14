; 内联汇编的语法极其复杂,为什么不使用字节熟悉的方式呢?
[bits 32]
; global jmpf
; jmpf:
; 现在的栈:
; IP
; CS
; 形式上的“返回地址”
; add esp, 4
; retf
global idle:
idle:
; sti
jmp $