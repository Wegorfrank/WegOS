# 我把loader安排在1号扇区
# 注意不能只写1个扇区
__loader__: loader.bin
	dd if=loader.bin of=$(WEGOS_PATH)/WEG.vhd bs=512 count=32 seek=1 conv=notrunc

loader.bin: loader.out
	$(TOOL_PREFIX)objcopy $(OBJCOPY_FLAGS) $(<) $(@)

# -Ttext=0x8000的作用:
# 让程序在链接的时候能够正确实现重定位
loader.out: start.o loader_16.o loader_32.o
	$(TOOL_PREFIX)ld $(LINK_FLAGS) -Ttext=0x8000 -o $(@) $(^)

start.o: start.s
	nasm $(<) -f elf -o $(@) -l $(basename $(<)).lst

loader_16.o: loader_16.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_16) $(<) -o $(@)

loader_32.o: loader_32.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

.PHONY: clean
clean:
	-rm loader_16.o
	-rm loader_32.o
	-rm start.o
	-rm start.lst
	-rm loader.out
	-rm loader.bin
	-rm loader_dis.txt

# objdump的选项要加上-d
# 也不知道李述铜的makefile是怎么跑起来的
.PHONY: debug
debug:
	-rm loader_dis.txt
	-$(TOOL_PREFIX)objdump $(DEBUG_DUMP_FLAGS_16) -d loader.out > loader_dis.txt