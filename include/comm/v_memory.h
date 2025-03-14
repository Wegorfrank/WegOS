#define DISK_SWAP_START_SECTOR 1030 // 我的磁盘太小了,1030只是一个测试值

#define SWAP_SECTOR_GET(x) x >> 3                        // 学习Linux,把交换区地址也保存至页表当中
#define SWAP_SECTOR_SET(x, sector) ((x) = (sector) << 3) // 顺便把存在位设置成0

int swap_alloc(void);
void swap_free(void);

void swap_out(unsigned int cr3, unsigned int useless);      // 换出的一定是当前进程的页吗?非也!cr3参数必须传入
void swap_in(unsigned int useful, unsigned short flags);    // 和swap_out不同,换入的一般只会是当前进程的页
int swap_sector_of(unsigned int cr3, unsigned int virtual); // 页不存在.获取指定页所在的交换区扇区号
void swap_sector_set(unsigned int cr3, unsigned int virtual, int sector);