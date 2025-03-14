; 为什么每个模块的入口点都要用汇编来写?
; 因为只有汇编才能够精准控制首条指令

; 汇编为ELF格式,不能用org伪指令了
; org 0x8000
extern loader_entry
extern load_kernel

; 本文件将会使用elf编译,所以必须指定操作尺寸
[bits 16]
global _start ; 这是为了避免0x8048000的编译警告
global kernel_datasg_init_16
_start:
call loader_entry
kernel_datasg_init_16:
; 可算是回来了
; 现在因为编译器的垃圾特性,堆栈与上条指令之前相比已经是不平衡的,之后要格外注意这个问题
; 设置相关的数据段寄存器
mov ax, 0x10
mov ds, ax
mov fs, ax
mov es, ax
mov gs, ax
; 注意:由于是通过段内调用的方式回到这里的,所以当前的默认操作尺寸仍然是16位!
jmp 0x8:kernel_datasg_init_32

[bits 32]
extern load_kernel
kernel_datasg_init_32:
; 现在!已经是32位的操作尺寸!我向我熟悉的环境迈进了一大步!
; 当务之急是把内核部分加载进来
call load_kernel
; 默认操作尺寸为32位时,编译器在函数调用上终于不抽风了
; 直接跳到0x100000地址处执行
jmp 0x8:0x100000