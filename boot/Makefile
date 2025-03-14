# 要适应Makefile工作环境

__boot__: start.bin
	dd if=start.bin of=$(WEGOS_PATH)/WEG.vhd bs=512 count=1 seek=0 conv=notrunc

# kernel_final.bin: kernel_final.out
# 	$(TOOL_PREFIX)objcopy $(OBJCOPY_FLAGS) kernel_final.out kernel_final.bin

# kernel_final.out: kernel.o entry.o
# 	$(TOOL_PREFIX)ld $(LINK_FLAGS) kernel.o entry.o -o kernel_final.out

start.bin: start.s
	nasm $(<) -o $(@) -l $(basename $(<)).lst

# entry.o: entry.c
# 	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_16) entry.c -o entry.o

.PHONY: clean
clean:
	-rm start.bin
	-rm start.lst

.PHONY: debug
debug: