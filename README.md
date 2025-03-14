# WegOS

一个实验性的简易操作系统，实现了基础进程管理、内存管理和自定义文件系统。

## ✨ 特性

- **引导方式**：传统磁盘引导（MBR）
- **内存管理**：基础实现（功能尚不完善）
- **文件系统**：自定义非主流文件系统，支持：
  - `sys_read`/`sys_write`系统调用
  - 独创的文件管理方案
- **架构支持**：x86及以上的英特尔架构

## 📁 目录结构
WegOS/
├── boot/ # 主引导扇区程序
├── loader/ # 二级引导加载器
├── kernel/ # 内核核心代码
└── include/ # 公共头文件

复制

## 🛠️ 构建指南

### 依赖项
- `x86_64-elf-gcc` 工具链
- `nasm` 汇编器
- `make` 构建工具
- `dd` 磁盘映像工具
- Bochs/QEMU 虚拟机（推荐Bochs）

### 构建步骤
1. 创建虚拟磁盘：
   ```bash
   cd /path/to/WegOS
   dd if=/dev/zero of=WEG.vhd bs=1M count=128  # 创建128MB磁盘映像
编译系统：

bash
复制
make        # 常规编译
make clean  # 清理构建产物
make debug  # 生成调试信息（需先完成常规编译）
运行系统：

bash
复制
bochs -f bochsrc.txt  # 需预先配置Bochs配置文件
💡 提示：如需自定义磁盘文件名，需修改Makefile中的相关配置

❗ 已知限制
内存管理模块功能不完整

不支持主流文件系统（FAT/NTFS/ext等）

仅支持传统BIOS引导
