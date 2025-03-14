# 测试文件:将二进制文件写入磁盘的指定扇区以测试读取情况
__test__: block_1026.bin block_1027.bin
	dd if=block_1026.bin of=$(WEGOS_PATH)/WEG.vhd bs=512 count=512 seek=1026 conv=notrunc
	dd if=block_1027.bin of=$(WEGOS_PATH)/WEG.vhd bs=512 count=512 seek=1027 conv=notrunc

block_1026.bin: block_1026.s
	nasm -o block_1026.bin block_1026.s

block_1027.bin: block_1027.s
	nasm -o block_1027.bin block_1027.s

clean:
	-rm block_1026.bin
	-rm block_1027.bin