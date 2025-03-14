// 我决定先实现文件系统
#include "fs.h"
#include "string.h"
#include "console.h"
#include "bit_allocation.h"

bitmap_t file_alloc_bitmap;
char fs_alloc_table[(1024 - 640) * 512]; // 如果内核存放的扇区改了,这个数组的大小也需要跟着变

file_t opened_files[1024]; // 系统的打开文件表!

void fs_init(void)
{
    bit_bind(&file_alloc_bitmap, fs_alloc_table, (1024 - 640) * 512 * 8);
#ifdef FS_INIT
    // 认定为首次启动操作系统,需要专门初始化磁盘块
    for (int i = 0; i < SECTOR_WITH_ROOT_FCB; i++) // 1024号磁盘块是指定用来存放根目录的FCB的,1025会被指定用来存放根目录的数据,也是处于被占用状态
        bit_free(&file_alloc_bitmap, i, true);
    root_init();
#endif
}

void load_alloc_table(void)
{
    read_disk(KERNEL_DEST + KERNEL_MAX_SIZE, 1024 - 640, (uint8_t *)file_alloc_bitmap.bits); // 开机时立马把文件分配位图读入内存
}

void save_alloc_table(void)
{
    write_disk(KERNEL_DEST + KERNEL_MAX_SIZE, 1024 - 640, (uint8_t *)file_alloc_bitmap.bits); // 写回
}

int file_alloc(void)
{
    bit_alloc(&file_alloc_bitmap, false);
}

// 本函数调用完毕后,1024号磁盘块当中应有正确的根目录的FCB并且根目录的数据部分放在了1025块
void root_init(void)
{
    block_t block_with_root_fcb;
    block_with_root_fcb.next_sector = 0xFFFFFFFF;
    file_t *root_fcb = (file_t *)&block_with_root_fcb;
    strcpy(root_fcb->file_name, "root");
    root_fcb->file_type = FILE_TYPE_DIRECTORY;
    root_fcb->first_sector = root_fcb->last_sector = 1025; // 所以,其实1025号是写死的..之所以专门在1024号放root的FCB是出于可读性的考虑
    root_fcb->sector_count = 1;
    bit_free(&file_alloc_bitmap, 1024, true);
    bit_free(&file_alloc_bitmap, 1025, true);
#ifdef _DEBUG
    root_fcb->file_size = sizeof(file_t); // 不专门对根目录的文件数作初始化
#endif
    write_disk(1024, 1, (uint8_t *)&block_with_root_fcb);
}

// 指定目录下查找文件.首个参数是目录的FCB.仍然使用指针传参的方式取得结果
// 返回值用于判断有没有找到
// p_index用于获取到文件是父目录下的第几个文件(传入NULL表示不需要这一信息)
file_result_t dir_search_file(file_t *p_dir, file_t *p_target, char file_name[], int *p_index)
{
    file_t files[512]; // 实现内存分配之前的权宜之计
    read_all(p_dir, (uint8_t *)files);
    for (int i = 0; i < p_dir->file_size / sizeof(file_t); i++) // dir.file_size / sizeof(block_t)代表文件数
    {
        if (strcmp(files[i].file_name, file_name) == 0)
        {
            *p_target = files[i];
            if (p_index)
                *p_index = i;
            // printf("FOUND\n");
            return FILE_RESULT_SUCCESS;
        }
    }
    // printf("NOT FOUND\n");
    return FILE_RESULT_NOT_EXISTING;
}

void read_all(file_t *p_file, uint8_t *buf)
{
#ifdef DEPRECATED
    uint32_t file_sector = p_file->first_sector;
    while (file_sector != (uint32_t)(-1))
    {
        read_disk(file_sector, 1, buf);
        file_sector = ((block_t *)buf)->next_sector;
        buf += VALID_DATA_SIZE_IN_BLOCK;
    }
#else
    // 根据新的逻辑编写新的读文件函数
    file_cached_t file_cached;
    file_cached.file = *p_file;
    file_cached.io_state.read_sector = p_file->first_sector;
    file_cached.io_state.read_seek = 0;
    // 有人提出:怎么调用sys_read之前还需要专门了解file_cached_t的内部结构啊?太麻烦了?
    // 我的看法是:这就和使用iret来切换特权级是一个道理
    sys_read(&file_cached, buf, p_file->file_size);
#endif
}

// 文件路径参考格式:root/usr/lib/libc/
// 除此以外的任何格式都无法被正常识别
file_result_t load_file_cached(const char *path, file_cached_t *p_file_cached)
{
    int length = strlen(path);
    char temp[FILE_NAME_MAXLENGTH]; // 文件名不会超过64字节,这段代码不需要因为内存管理而修改
    int j = 0;
    char root_buf[SECTOR_SIZE];                              // 虽然root的FCB很小,但是读磁盘仍然是以块为单位的,之前我把它读到一个很小的栈区导致中断向量表被破坏了
    read_disk(SECTOR_WITH_ROOT_FCB, 1, (uint8_t *)root_buf); // 根目录的FCB可以保证是1个扇区能够放得下的
    memcpy(&p_file_cached->file, root_buf, sizeof(file_t));
    for (int i = 0; i < length; i++)
    {
        if (path[i] == '/')
        {
            temp[j] = '\0';
            j = 0;
            // 以下是对每个文件名temp的处理逻辑
            if (dir_search_file(&p_file_cached->file, &p_file_cached->file, temp, &p_file_cached->path_state.index_in_parent_dir) == FILE_RESULT_NOT_EXISTING)
            {
                // 为了方便易用,要专门区分一下是文件本身没找到还是它的父目录没找到
                // 现在的区分标准:查看i>=length-1
                // i>=length-1说明这已经是在查看最后一级了
                return i >= length - 1 ? FILE_RESULT_NOT_EXISTING : FILE_RESULT_DIR_NOT_EXISTING;
            }
        }
        else
        {
            // 正常的字符
            temp[j++] = path[i];
        }
    }
    p_file_cached->io_state.read_sector = p_file_cached->io_state.write_sector = p_file_cached->file.first_sector;
    p_file_cached->io_state.read_seek = p_file_cached->io_state.write_seek = 0;
    if (strcmp(path, "") == 0) // 现在取得的是根目录的FCB!
        p_file_cached->path_state.has_parent = false;
    else
    {
        p_file_cached->path_state.has_parent = true;
        char parent_dir_name[FILE_PATH_MAXLENGTH];
        get_parent_dir_name(path, parent_dir_name);
        strcpy(p_file_cached->path_state.parent_dir_name, parent_dir_name);
    }
    return FILE_RESULT_SUCCESS;
}

void store_file_cached(file_cached_t *p_file_cached)
{
    file_cached_t parent_dir;
    if (load_file_cached(p_file_cached->path_state.parent_dir_name, &parent_dir) == FILE_RESULT_SUCCESS)
    {
        if (p_file_cached->path_state.has_parent)
        {
            sys_wseek(&parent_dir, sizeof(file_t) * p_file_cached->path_state.index_in_parent_dir);
            covering_write(&parent_dir, (char *)&p_file_cached->file /*p_file_cached*/, sizeof(file_t));
        }
        else
        {
            // 根目录FCB写回
            // 因为根目录FCB没法锁定一个具体的路径,所以只能用土办法写回了
            char temp[SECTOR_SIZE];
            memcpy((void *)temp, (void *)&p_file_cached->file, sizeof(file_t));
            write_disk(SECTOR_WITH_ROOT_FCB, 1, temp); // SECTOR_WITH_ROOT_FCB号扇区当中只可能有一个FCB,所以就不需要先读再改后写入了
        }
    }
}

// 系统调用的思想是要偏向底层,所以我设计这样一个比较底层的创建文件功能
file_result_t sys_create(file_cached_t *p_dir_cached, char file_name[], file_type_t file_type)
{
    file_t new_file;
    strcpy(new_file.file_name, file_name);
    new_file.file_size = 0;
    new_file.file_type = file_type;
    new_file.first_sector = new_file.last_sector = file_alloc();
    // 新申请的那个磁盘需要进行初始化
    block_t new_file_block;
    new_file_block.next_sector = 0xFFFFFFF;
    write_disk(new_file.first_sector, 1, (char *)&new_file_block); // 因为是新的扇区,所以也不需要先读后写
    new_file.sector_count = 1;
    sys_write(p_dir_cached, (char *)&new_file, sizeof(file_t)); // 正常用户不可能乱改目录FCB的读写指针,所以无需提前设置wseek
}

// 本函数只负责把磁盘数据原封不动复制到内存,对于字符串,不负责在buf的尾部补'\0'!
file_result_t sys_read(file_cached_t *p_file_cached, char *buf, int length)
{
    if (p_file_cached->io_state.read_seek + length <= p_file_cached->file.file_size) // 事前判断有没有越界
    {
        char temp[SECTOR_SIZE];
        // 暴力解法
        // printf("A read_disk,sector=");
        // PRINT_HEX(p_file_cached->io_state.read_sector);
        // printf("\n");
        read_disk(p_file_cached->io_state.read_sector, 1, (uint8_t *)temp);
        if (NTH_BLOCK(p_file_cached->io_state.read_seek + length) - NTH_BLOCK(p_file_cached->io_state.read_seek) == 0)
        {
            // 即使上一次刚好读完某个扇区,那应该走这个逻辑
            // 只需要读入零碎的一个扇区
            memcpy(buf, temp + IN_BLOCK_OFFSET(p_file_cached->io_state.read_seek), length);
            p_file_cached->io_state.read_seek += length;
            length = 0;                 // 标记:读完了
            return FILE_RESULT_SUCCESS; // 既然已经读完了,那就直接返回算了,免得莫名其妙的错误
        }
        else
        {
            // 即使刚好把某个扇区读完,也需要把读写指针和读写扇区后移
            int remain = VALID_DATA_SIZE_IN_BLOCK - IN_BLOCK_OFFSET(p_file_cached->io_state.read_seek);
            memcpy(buf, temp + IN_BLOCK_OFFSET(p_file_cached->io_state.read_seek), remain);
            p_file_cached->io_state.read_seek += remain;
            p_file_cached->io_state.read_sector = ((block_t *)temp)->next_sector;
            length -= remain;
            buf += remain;
        }
        // 在新起点上读入新的数据
        int sectors = length / VALID_DATA_SIZE_IN_BLOCK;
        // 读入剩余的"整"的数据
        for (int i = 0; i < sectors; i++)
        {
            // printf("B read_disk,sector=");
            // PRINT_HEX(p_file_cached->io_state.read_sector);
            // println();
            read_disk(p_file_cached->io_state.read_sector, 1, buf);
            p_file_cached->io_state.read_sector = ((block_t *)buf)->next_sector;
            p_file_cached->io_state.read_seek += VALID_DATA_SIZE_IN_BLOCK;
            buf += VALID_DATA_SIZE_IN_BLOCK;
        }
        // "零"的数据
        int scattered = IN_BLOCK_OFFSET(length);
        // printf("C read_disk,sector=");
        // PRINT_HEX(p_file_cached->io_state.read_sector);
        // println();
        read_disk(p_file_cached->io_state.read_sector, 1, temp);
        memcpy(buf, temp, scattered);
        return FILE_RESULT_SUCCESS;
    }
    else
        return FILE_RESULT_OUT_OF_INDEX;
}

void covering_write(file_cached_t *p_file_cached, char *buf, int length) // 覆盖式写入.该函数会改变文件的读写指针
{
    // 暂不考虑写越界的情况
    char temp[SECTOR_SIZE];
    read_disk(p_file_cached->io_state.write_sector, 1, (uint8_t *)temp); // 如果是在扇区的中间写入,也需要先读入然后再写入..这就是写操作的特点
    if (NTH_BLOCK(p_file_cached->io_state.write_seek + length) - NTH_BLOCK(p_file_cached->io_state.write_seek) == 0)
    {
        // 只用写入零散的一个扇区
        memcpy(temp + IN_BLOCK_OFFSET(p_file_cached->io_state.write_seek), buf, length);
        p_file_cached->io_state.write_seek += length;
        write_disk(p_file_cached->io_state.write_sector, 1, temp); // 写回
        return;
    }
    else
    {
        int remain = VALID_DATA_SIZE_IN_BLOCK - IN_BLOCK_OFFSET(p_file_cached->io_state.write_seek);
        memcpy(temp + IN_BLOCK_OFFSET(p_file_cached->io_state.write_seek), buf, remain);
        write_disk(p_file_cached->io_state.write_sector, 1, temp);
        p_file_cached->io_state.write_seek += remain;
        p_file_cached->io_state.write_sector = ((block_t *)temp)->next_sector;
        length -= remain;
        buf += remain;
    }
    // 在新起点上写入新的数据
    int sectors = length / VALID_DATA_SIZE_IN_BLOCK;
    // 写入剩余的"整"的数据
    for (int i = 0; i < sectors; i++)
    {
        write_disk(p_file_cached->io_state.write_sector, 1, buf);
        p_file_cached->io_state.write_sector = ((block_t *)buf)->next_sector;
        p_file_cached->io_state.write_seek += VALID_DATA_SIZE_IN_BLOCK;
        buf += VALID_DATA_SIZE_IN_BLOCK;
    }
    // "零"的数据
    // 怎么能直接就不分青红皂白的写呢?要先保存原有数据
    int scattered = IN_BLOCK_OFFSET(length);
    read_disk(p_file_cached->io_state.write_sector, 1, temp);
    memcpy(temp, buf, scattered);
    write_disk(p_file_cached->io_state.write_sector, 1, temp);
    p_file_cached->io_state.write_seek += scattered; // 记得再次更新写指针
}

// 为了简洁起见,调用此函数之前,文件本身必须存在
// WegOS的文件盘块机制:文件申请额外的盘块不会导致文件大小的增加.文件大小缩减之后盘块也不会自动回收.
file_result_t file_expand(file_cached_t *p_file_cached, int need)
{
    for (int i = 0; i < need; i++)
    {
        int new_last_sector = file_alloc();
        int old_last_sector = p_file_cached->file.last_sector;
        block_t new_last_block, old_last_block;
        // 只支持"尾插法",不支持"头插法"
        read_disk(old_last_sector, 1, (uint8_t *)&old_last_block);
        old_last_block.next_sector = new_last_sector;
        write_disk(old_last_sector, 1, (uint8_t *)&old_last_block);
        read_disk(new_last_sector, 1, (uint8_t *)&new_last_block);
        new_last_block.next_sector = 0xFFFFFFFF;
        write_disk(new_last_sector, 1, (uint8_t *)&new_last_block);
        p_file_cached->file.last_sector = new_last_sector;
    }
    p_file_cached->file.sector_count += need;
    // FCB数据写回
    store_file_cached(p_file_cached);
}

// 读写指针默认是从文件开头起算
file_result_t sys_rseek(file_cached_t *p_file, int read_seek)
{
    if (read_seek < p_file->file.file_size)
    {
        int sector_index = NTH_BLOCK(read_seek); // 文件的第几个扇区?
        block_t file_block;
        int read_sector = p_file->file.first_sector;
        for (int i = 0; i < sector_index; i++)
        {
            read_disk(read_sector, 1, (uint8_t *)&file_block);
            read_sector = file_block.next_sector;
        }
        p_file->io_state.read_sector = read_sector;
        p_file->io_state.read_seek = read_seek;
        return FILE_RESULT_SUCCESS;
    }
    else
        return FILE_RESULT_OUT_OF_INDEX;
}

file_result_t sys_wseek(file_cached_t *p_file, int write_seek)
{
    int sector_index = NTH_BLOCK(write_seek); // 文件的第几个扇区?
    block_t file_block;
    int write_sector = p_file->file.first_sector;
    for (int i = 0; i < sector_index; i++)
    {
        read_disk(write_sector, 1, (uint8_t *)&file_block); // 即使是修改写指针也需要读盘
        write_sector = file_block.next_sector;
    }
    p_file->io_state.write_sector = write_sector;
    p_file->io_state.write_seek = write_seek;
    return FILE_RESULT_SUCCESS;
}

void sys_write(file_cached_t *p_file_cached, char *buf, int length)
{
    // 现在来实现真正的对用户友好的文件系统写入接口
    // 判断文件在写入新数据之后还能不能用旧有的盘块装下
    // 文件需要的盘块数目=NTH_BLOCK(文件大小)+1
    // 即使文件刚刚好装满一个盘块,也需要有一个额外的盘块以避免读写指针越界
    if (NTH_BLOCK(p_file_cached->file.file_size + length) + 1 > p_file_cached->file.sector_count)
    {
        // 装不下了!算一算需要多少个新的盘块
        int need = NTH_BLOCK(p_file_cached->file.file_size + length) + 1 - p_file_cached->file.sector_count;
#ifdef _DEBUG
        if (need <= 0)
            while (true)
                printf("DATA INCORRESPONDING WHEN CALCULATING NEW SECTORS\n");
#endif
        file_expand(p_file_cached, need);
    }
    // 可以这么做:将剩余部分全部读入内存,然后在内存当中追加,然后覆盖式写入.虽然这样会占用大量内存但是很简洁
    // 之后我再去优化算法
    char temp[4096]; //  char* temp = malloc(remain + length);
    int old_read_seek = p_file_cached->io_state.read_seek;
    int old_write_seek = p_file_cached->io_state.write_seek + length; // 这一变量名其实并不恰当..我只想说明这是过一会写指针应该被赋的值
    int remain = p_file_cached->file.file_size - p_file_cached->io_state.write_seek;
    sys_rseek(p_file_cached, p_file_cached->io_state.write_seek); // 强制让读指针和写指针同步
    read_remains(p_file_cached, temp);
    printf("A:");
    for (int i = 0; i < 64; i++)
        putchar(temp[i]);
    println();
    // 内存拼接分为两步,先把临时数组的元素往后挪,然后在前边填充buf的元素
    // memcpy((void *)(temp + length), (void *)temp, remain); // 错误做法!源字符串和目标字符串存在交叉的时候要慎用memcpy
    for (int i = remain - 1; i >= 0; i--)
        temp[length + i] = temp[i]; // 要从后往前复制
    printf("B:");
    for (int i = 0; i < 64; i++)
        putchar(temp[i]);
    println();
    memcpy((void *)temp, (void *)buf, length);
    printf("C:");
    for (int i = 0; i < 64; i++)
        putchar(temp[i]);
    println();
    covering_write(p_file_cached, temp, remain + length);
    sys_wseek(p_file_cached, old_write_seek); // 恢复原来的写指针
    sys_rseek(p_file_cached, old_read_seek);  // 恢复原来的读指针
    // FCB的数据会因为sys_write而发生改变
    p_file_cached->file.file_size += length;
    store_file_cached(p_file_cached);
}

void read_remains(file_cached_t *p_file_cached, char *buf)
{
    int remain = p_file_cached->file.file_size - p_file_cached->io_state.read_seek;
    sys_read(p_file_cached, buf, remain);
}

#ifdef _DEBUG
void print_file_cached_info(file_cached_t *p_file_cached)
{
    printf("{");
    printf("first sector:");
    PRINT_HEX(p_file_cached->file.first_sector);
    printf(",read sector:");
    PRINT_HEX(p_file_cached->io_state.read_sector);
    printf(",write sector:");
    PRINT_HEX(p_file_cached->io_state.write_sector);
    printf(",read seek:");
    PRINT_HEX(p_file_cached->io_state.read_seek);
    printf(",write seek:");
    PRINT_HEX(p_file_cached->io_state.write_seek);
    printf(",parent_dir_name:");
    printf(p_file_cached->path_state.parent_dir_name);
    printf(",index in parent dir:");
    PRINT_HEX(p_file_cached->path_state.index_in_parent_dir);
    printf("}");
}
#endif