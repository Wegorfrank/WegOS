; 汇编语言源程序都以.s结尾(这是约定)

; 以下程序会被加载到内存0x7C00位置
org 0x7C00

boot:;boot标号将等于0x7C00
; 必要的寄存器对齐工作
xor ax, ax
mov ds, ax
; 栈设置在0xA0000位置,这是为了在保证不破坏中断向量表、显存、ROM之类的数据结构前提下尽可能远离代码段
mov ax, 0x9000
mov ss, ax
; 把ax再清零一次吧，为了更好的可读性
xor ax, ax
mov ax, 0xFFFE
mov sp, ax
; ↑现在栈为空!
; 主引导扇区应该做什么呢?这不确定.但是可以肯定的是,它有义务把剩余部分从磁盘搬到内存.
; 搬到什么位置?0x8000位置肯定是最合适的.

; 这个操作系统是我做出来的!我有权向屏幕输出信息!
; 屏幕一行80(0x50)个字符,共25(0x19)行
; 先清屏
call cls

xor ax, ax
mov ds, ax

mov si, copyright_Welcome_to_WegOS
xor di, di
mov bx, 0x0B00
call show

mov si, copyright_The_OS_is_developed_by
mov di, 0x50*2 ;换行显示(乘二是因为要考虑颜色字节)
mov bx, 0x0B00
call show

call read_loader
jmp 0:0x8000 ;我写代码就和做数学题一样不细心,我一开始写成了0x8000:0

read_loader:;写死,读取到0x8000位置
mov ax, 0x800
mov ds, ax
; 端口号0x1F2是要读取的扇区数
; 端口号是也必须是16位
mov dx, 0x1F2
; 暂且认为32个扇区足够存放loader的代码了(写死)
mov al, 0x20
out dx, al
; LBA方式下扇区号占28位
; 端口号0x1F3是0-7位
mov dx, 0x1F3
mov al, 0x01
out dx, al
; 端口号0x1F4是8-15位
xor al, al
mov dx, 0x1F4
out dx, al
; 端口号0x1F5是16-23位
mov dx, 0x1F5
out dx, al
; 端口号0x1F6低4位是24-27位
; 端口号0x1F6的高4位很不好处理,直接记住填1110
mov al, 0xE0
mov dx, 0x1F6
out dx, al
; 端口号0x1F7是控制寄存器,用于发送读写命令
; 发送读命令应该写入0x20
mov dx, 0x1F7
mov al, 0x20
out dx, al
; 循环等待直到读取完成:0x1F7也是状态寄存器
waits:
in al, dx
and al, 0x88
cmp al, 0x08
jnz waits
; 端口号0x1F0是数据寄存器
mov cx, 0x2000 ;32个扇区(写死)
mov bx, 0
mov dx, 0x1F0
continue_to_read:
in ax, dx
mov [bx], ax
add bx, 2
loop continue_to_read
ret

; 临时制作一个字符串显示例程(所有的函数都写到最后!以免误执行)
show:
; 传参:
; ds:si->字符串开始地址
; 0xB800:di->目标位置
; bh->显示颜色
; bl->字符串结束标志
mov ax, 0xB800 ;本例程会破坏寄存器ax!
mov es, ax
show_loop:
cmp byte [si], bl
je show_end
show_single_char:
movsb
mov [es:di], bh
inc di
jmp show_loop
show_end:
ret

cls:
mov ax, 0xB800
mov ds, ax
mov cx, 0x50*0x19
xor di, di
put_blank:
mov byte [di], ' '
add di, 0x2
loop put_blank
ret

copyright_Welcome_to_WegOS:
db "Welcome to WegOS.", 0 ;长度17=0x11
copyright_The_OS_is_developed_by:
db "The OS is developed independently by Wenge Wang in China.", 0 ;长度57=0x39

times (510-($-$$)) db 0
; Magic number
boot_end:;此标号应该用不到,单纯是可读性的要求
db 0x55, 0xAA