; 一个模块的入口统一命名为start.s!

[bits 32]
global _start
extern main, kernel_stack
_start:
; 进入main函数
mov ax, 0x10
; 要先进行栈和数据段的基地址对齐,不然会发生意想不到的错误
mov ss, ax
; 代码能跑就别乱动!
; esp也需要正确的设置以保证栈区不会和其他数据区冲突
; 不然这导致的错误将更加难以排查
mov esp, kernel_stack
; 我已经经历过这种难以排查的错误了..
add esp, 4096
; 我已经等不及了,快点端上来吧
call main