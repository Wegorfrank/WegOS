[bits 32]
global keyboard_interrupt_handler,print_debug_info
extern on_keyboard
keyboard_interrupt_handler:
pushad
push ds
call on_keyboard
pop ds
popad
iret
print_debug_info: