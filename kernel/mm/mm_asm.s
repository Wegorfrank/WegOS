[bits 32]
global page_fault_handler
extern on_page_fault
page_fault_handler:
pushad
push ds
; 精准堆栈获取错误码
mov eax, [esp+0x24]
mov ebx, cr2
push eax ; 从右往左入栈
push ebx
call on_page_fault
add esp, 8 ; gcc的C调用协议要求由调用者平衡堆栈
pop ds
popad
add esp, 4
iret