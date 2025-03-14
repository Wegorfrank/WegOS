#ifndef EXECVE_H
#define EXECVE_H

// 由于我完全搞不懂ELF文件格式是怎么回事,所以我使用一个更加通用的文件结构
// void task_mmap_init(unsigned int cr3,
//                     unsigned int code_virtual, unsigned int code_length, unsigned int code_buf,
//                     unsigned int data_virtual, unsigned int data_length, unsigned int data_buf);
void sys_execve(const char *path);

#endif