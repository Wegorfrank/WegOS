#ifndef FS_H
#define FS_H

#include "stdint.h"
#include "disk.h"
#define FILE_NAME_MAXLENGTH 64
#define FILE_PATH_MAXLENGTH 512
#define SECTOR_WITH_ROOT_FCB 1024 // 根目录->1024号扇区
#define IN_BLOCK_OFFSET(x) ((x) % (SECTOR_SIZE - sizeof(uint32_t)))
#define NTH_BLOCK(x) ((x) / (SECTOR_SIZE - sizeof(uint32_t)))
#define VALID_DATA_SIZE_IN_BLOCK (SECTOR_SIZE - sizeof(uint32_t))

// 文件路径的处理模板(但是太复杂了而且容易出错,我应该不会用到它...)
#define PATH_ANALYSE(path, handler)   \
    int length = strlen(path), j = 0; \
    char temp[FILE_NAME_MAXLENGTH];   \
    for (int i = 0; i < length; i++)  \
    {                                 \
        if (path[i] == '/')           \
        {                             \
            temp[j] = '\0';           \
            j = 0;                    \
            handler(temp);            \
        }                             \
        else                          \
        {                             \
            temp[j++] = path[i];      \
        }                             \
    }

typedef enum file_type_t
{
    FILE_TYPE_NORMAL,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_SOFT_LINK // 软链接
} file_type_t;

// FCB
typedef struct file_t
{
    char file_name[FILE_NAME_MAXLENGTH];              // 文件名
    uint32_t first_sector, last_sector, sector_count; // 起始扇区,结束扇区,一共占用多少个扇区(现在已经专门记录了很多个为了时间效率设置的字段,一定要保证数据一致性)
    int file_size;                                    // 单位:字节
    file_type_t file_type;
} file_t;

typedef struct file_io_state_t
{
    int read_seek, write_seek; // 读写指针(偏移自文件开始计算)
    int read_sector, write_sector;
} file_io_state_t;

typedef struct file_cached_t
{
    file_t file;
    file_io_state_t io_state;
    struct file_path_state_t
    {
        char parent_dir_name[FILE_PATH_MAXLENGTH];
        int index_in_parent_dir;
        bool has_parent;
    } path_state;
} file_cached_t;

typedef struct block_t
{
    uint8_t data[SECTOR_SIZE - sizeof(uint32_t)];
    uint32_t next_sector; // -1表示没有了
} block_t;

typedef enum file_result_t
{
    FILE_RESULT_SUCCESS,
    FILE_RESULT_NO_MORE_DISK_SPACE,
    FILE_RESULT_ALREADY_EXISTING,
    FILE_RESULT_NOT_EXISTING,
    FILE_RESULT_DIR_NOT_EXISTING,
    FILE_RESULT_OUT_OF_INDEX,
    FILE_RESULT_TYPE_ERROR,
} file_result_t;

// file_t file_empty = {0};
void load_alloc_table(void);
void save_alloc_table(void);
int file_alloc(void);

file_result_t dir_search_file(file_t *p_dir, file_t *p_target, char file_name[], int *p_index);
file_result_t load_file_cached(const char *path, file_cached_t *p_file_cached);
void store_file_cached(file_cached_t *p_file_cached); // 文件节点已经存在,并且不可能越界,所以返回值是void
void read_all(file_t *file, uint8_t *buf);
void fs_init(void);

// 主要文件系统接口
// file_t sys_open(const char *path, int flags, ...); // 这又不是POSIX的要求,我完全可以定义我自己的返回值类型
file_result_t sys_read(file_cached_t *p_file, char *buf, int length);
void sys_write(file_cached_t *p_file_cached, char *buf, int length);
void covering_write(file_cached_t *p_file_cached, char *buf, int length);
void read_remains(file_cached_t *p_file_cached, char *buf);
file_result_t sys_create(file_cached_t *p_dir_cached, char file_name[], file_type_t file_type); // 其实目录FCB的io_state和path信息根本用不到...但是为了编程方便以及可扩展性把这些属性也传入
file_result_t sys_rseek(file_cached_t *p_file, int read_seek);
file_result_t sys_wseek(file_cached_t *p_file, int read_seek);
// int sys_write(int file, char *ptr, int len);
// int sys_lseek(int file, int ptr, int dir);
// int sys_close(int file);

void root_init(void);                                              // 直观的函数名:根目录初始化
file_result_t file_expand(file_cached_t *p_file_cached, int need); // 文件占用盘块数目的动态扩充.

#ifdef _DEBUG
void print_file_cached_info(file_cached_t *p_file_cached);
#endif

#endif