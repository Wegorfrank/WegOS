# 示例命令(仅供参考)
# x86_64-elf-gcc -c -m16 -march=i386 entry.c -o entry.o
# nasm kernel.s -f elf -o kernel.o
# x86_64-elf-ld -m elf_i386 kernel.o entry.o -o kernel_final.out
# x86_64-elf-objcopy -I elf32-i386 -S -R ".eh_frame" -R ".comment" -O binary kernel_final.out kernel_final.bin
# dd if=kernel_final.bin of=LEARN.vhd bs=512 count=2 seek=0 conv=notrunc

WEGOS_PATH = D:/IntegratedCode/OSCode/WegOS
export WEGOS_PATH

TOOL_PREFIX = x86_64-elf-
export TOOL_PREFIX

# 指定多个头文件搜索路径时应该用多个I选项
C_COMPILE_FLAGS_16 = -g -c -O0 -m16 -march=i386 -fno-builtin -I$(WEGOS_PATH)/include -I$(WEGOS_PATH)/include/comm -D_DEBUG -DFS_INIT
C_COMPILE_FLAGS_32 = -g -c -O0 -m32 -march=i386 -fno-builtin -I$(WEGOS_PATH)/include -I$(WEGOS_PATH)/include/comm -D_DEBUG -DFS_INIT
export C_COMPILE_FLAGS_16 C_COMPILE_FLAGS_32

# 这个变量比较短,并且我已经理解了它的意思,其实不用导出的...
ELF_ASM_FLAGS = -f elf
export ELF_ASM_FLAGS

LINK_FLAGS = -m elf_i386
export LINK_FLAGS

DEBUG_DUMP_FLAGS_16 = -x -S -m i8086
DEBUG_DUMP_FLAGS_32 = -x -S -m i386
export DEBUG_DUMP_FLAGS16 DEBUG_DUMP_FLAGS_32

# OBJCOPY_FLAGS = -I elf32-i386 -S -R ".eh_frame" -R ".comment" -O binary
OBJCOPY_FLAGS = -I elf32-i386 -S -R ".eh_frame" -R ".comment" -O binary
export OBJCOPY_FLAGS

# 强制每次生成的时候自带调试信息
WegOS: __boot__ __loader__ __kernel__ __test__ debug

__boot__:
	$(MAKE) -C boot

__loader__:
	$(MAKE) -C loader

__kernel__:
	$(MAKE) -C kernel

__test__:
	$(MAKE) -C test

.PHONY: clean
clean:
	-$(MAKE) clean -C boot
	-$(MAKE) clean -C loader
	-$(MAKE) clean -C kernel
	-$(MAKE) clean -C test

.PHONY: debug
debug:
	-$(MAKE) debug -C boot
	-$(MAKE) debug -C loader
	-$(MAKE) debug -C kernel