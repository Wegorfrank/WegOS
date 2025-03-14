# 目前我使用的方法是把剩下的全部代码一股脑进行链接
# 不知道这是不是一个比较好的做法
# 但是绝对是一个正确的且省事的做法

# 我把kernel安排在128号扇区
# 注意不能只写1个扇区
__kernel__: kernel.bin
	dd if=kernel.bin of=$(WEGOS_PATH)/WEG.vhd bs=512 count=512 seek=128 conv=notrunc

kernel.bin: kernel.out
	$(TOOL_PREFIX)objcopy $(OBJCOPY_FLAGS) $(<) $(@)

# 让程序在链接的时候能够正确实现重定位
kernel.out: init/start.o init/main.o \
 interrupt/interrupt.o interrupt/interrupt_start.o \
time/time.o time/time_asm.o \
dev/console.o dev/dev_asm.o dev/keyboard.o \
proc/proc_asm.o proc/task.o proc/sched.o proc/semaphore.o \
lib/string.o lib/bit.o lib/data_struct.o lib/bit_allocation.o \
mm/memory.o mm/v_memory.o mm/mm_asm.o mm/execve.o\
fs/fs.o
	$(TOOL_PREFIX)ld $(LINK_FLAGS) -Ttext=0x100000 -o $(@) $(^)

# kernel的有关文件链接时记得把目录名带上
init/start.o: init/start.s
	nasm $(<) -f elf -o $(@) -l $(basename $(<)).lst
	
init/main.o: init/main.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

interrupt/interrupt.o: interrupt/interrupt.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

interrupt/interrupt_start.o: interrupt/interrupt_start.s
	nasm $(<) -f elf -o $(@) -l $(basename $(<)).lst

time/time.o: time/time.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

time/time_asm.o: time/time_asm.s
	nasm $(<) -f elf -o $(@) -l $(basename $(<)).lst

dev/console.o: dev/console.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

dev/dev_asm.o: dev/dev_asm.s
	nasm $(<) -f elf -o $(@) -l $(basename $(<)).lst

dev/keyboard.o: dev/keyboard.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

# 就还是先用正常的链接方式,别瞎搞
# 我现在可不清楚ar库链接命令的具体用法以及需要加什么链接标志
lib/string.o: lib/string.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

lib/bit.o: lib/bit.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

lib/bit_allocation.o: lib/bit_allocation.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

lib/data_struct.o: lib/data_struct.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

proc/proc_asm.o: proc/proc_asm.s
	nasm $(<) -f elf -o $(@) -l $(basename $(<)).lst

proc/task.o: proc/task.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

proc/sched.o: proc/sched.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

proc/semaphore.o: proc/semaphore.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

mm/memory.o: mm/memory.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

mm/execve.o: mm/execve.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

mm/v_memory.o: mm/v_memory.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

mm/mm_asm.o: mm/mm_asm.s
	nasm $(<) -f elf -o $(@) -l $(basename $(<)).lst

fs/fs.o: fs/fs.c
	$(TOOL_PREFIX)gcc $(C_COMPILE_FLAGS_32) $(<) -o $(@)

.PHONY: clean
clean:
	-rm init/start.o
	-rm init/start.lst
	-rm init/main.o
	-rm interrupt/interrupt.o
	-rm interrupt/interrupt_start.o
	-rm dev/keyboard.o
	-rm dev/console.o
	-rm dev/dev_asm.o
	-rm kernel.bin
	-rm kernel.out

.PHONY: debug
debug:
	-rm kernel_dis.txt
	-$(TOOL_PREFIX)objdump $(DEBUG_DUMP_FLAGS_16) -d kernel.out > kernel_dis.txt