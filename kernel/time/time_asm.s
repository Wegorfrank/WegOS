[bits 32]
extern on_timer
global timer_interrupt_handler
timer_interrupt_handler:
; 又一个底层bug:
; 中断处理程序一定要保存易变寄存器,不然会出现极其难以排查的错误
; 现在我对于计组教材上的中断处理过程有了更深入的理解
pushad
push ds
call on_timer
pop ds
popad
iret