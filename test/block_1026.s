[bits 32]
here:
inc byte [0xB8000] ; 测试程序的效果要尽可能明显
cli
jmp here
; 和主引导扇区结束标记一样,我也要根据文件系统写入文件结束标记
times (508-($-$$))db 0
dd 1027