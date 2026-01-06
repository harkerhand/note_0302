# 操作系统实验报告：文件系统的实现

## 基本信息

* **姓名**：巩皓锴
* **学号**：09023321
* **日期**：2026年1月6日

## 实验内容

通过实验完整了解文件系统实现机制。

## 实验目的

实现具有设备创建、分区格式化、注册文件系统、文件夹操作、文件操作功能的完整文件系统。

## 设计思路

本实验采用 **FUSE (Filesystem in Userspace)** 架构。FUSE 允许非特权用户在不修改内核代码的情况下创建自己的文件系统。

1. **分层架构**：内核 FUSE 模块负责接收系统调用并将其转发至用户态，本程序（FullMemoryFS）通过继承 `fuse.Operations` 类来响应这些请求。
2. **存储机制**：由于是内存文件系统，所有数据均不持久化。使用 Python 字典 `self.fd` 存储文件的 **Inode/元数据**，使用 `self.data` 存储文件的 **Block/实际内容**。
3. **路径映射**：以路径字符串（如 `/test.txt`）作为键值，直接关联其属性和内容。

## 主要数据结构及其说明

* **`self.fd` (元数据字典)**：
* **键**：文件绝对路径（String）。
* **值**：包含文件属性的字典。常用属性包括：
* `st_mode`: 文件类型（目录或普通文件）及权限。
* `st_size`: 文件大小（字节）。
* `st_nlink`: 硬链接数。
* `st_atime`/`st_mtime`/`st_ctime`: 访问/修改/创建时间。




* **`self.data` (数据字典)**：
* **键**：文件绝对路径。
* **值**：二进制数据流（Bytes），存储文件实际写入的内容。


* **`self.next_fd`**：一个递增的整数，模拟内核分配的文件描述符。

## 源程序

```python
import logging
import time
import os
import errno
from stat import S_IFDIR, S_IFREG # 引入文件类型常量
from fuse import FUSE, Operations, FuseOSError

# --- 内存文件系统类 ---
class FullMemoryFS(Operations):
    """
    一个实现所有核心操作的内存文件系统。
    数据和元数据仅存在于内存中，程序终止数据即丢失。
    """
    def __init__(self):
        # logging.info("Filesystem initialized.")
        self.fd = {}   # 文件/目录的元数据字典: {path: attributes}
        self.data = {} # 文件内容字典: {path: bytes_content}
        self.root_init()
        self.next_fd = 0 # 用于分配文件描述符 (File Handler)

    def root_init(self):
        """初始化根目录的元数据"""
        now = time.time()
        self.fd['/'] = dict(
            st_mode=(S_IFDIR | 0o755), 
            st_ctime=now, 
            st_mtime=now, 
            st_atime=now, 
            st_nlink=2, 
            st_uid=os.getuid(), 
            st_gid=os.getgid(), 
            st_size=0
        )

    # --- 1. 元数据操作 (Metadata Operations) ---

    def getattr(self, path, fh=None):
        """获取文件或目录的属性 (stat)。必选。"""
        # logging.info(f"getattr: {path}")
        if path not in self.fd:
            # 文件不存在，返回标准错误
            raise FuseOSError(errno.ENOENT)
        return self.fd[path]


    # --- 2. 目录操作 (Directory Operations) ---

    def readdir(self, path, fh):
        """列出目录内容 (ls)。必选。"""
        # logging.info(f"readdir: {path}")
        # 必须返回 . 和 ..
        yield '.'
        yield '..'
        
        # 查找所有以当前路径为前缀且是下一级子项的路径
        for name in self.fd:
            if name.startswith(path):
                # 提取相对路径，并移除前缀 '/'
                sub_path = name[len(path):]
                
                if sub_path.startswith('/'):
                    sub_path = sub_path[1:]
                
                if '/' not in sub_path and sub_path:
                    # 确保只返回一级子目录/文件
                    yield sub_path

    def mkdir(self, path, mode):
        """创建目录 (mkdir)。"""
        # logging.info(f"mkdir: {path}")
        now = time.time()
        # 创建新的目录元数据
        self.fd[path] = dict(
            st_mode=(S_IFDIR | mode), 
            st_ctime=now, 
            st_mtime=now, 
            st_atime=now, 
            st_nlink=2, # 新目录链接数为 2 (. 和 ..)
            st_uid=os.getuid(), 
            st_gid=os.getgid(), 
            st_size=0
        )
        # 增加父目录的链接数 (NFS 规范要求)
        parent_dir = os.path.dirname(path)
        if parent_dir in self.fd:
             self.fd[parent_dir]['st_nlink'] += 1

    def rmdir(self, path):
        """删除目录 (rmdir)。"""
        # 实际实现中需要检查目录是否为空
        # logging.info(f"rmdir: {path}")
        del self.fd[path]
        
        # 减少父目录的链接数
        parent_dir = os.path.dirname(path)
        if parent_dir in self.fd:
             self.fd[parent_dir]['st_nlink'] -= 1

    # --- 3. 文件创建/删除/重命名 (File Structure Operations) ---

    def create(self, path, mode, fi=None):
        """创建新文件 (touch, open(O_CREAT))。"""
        # logging.info(f"create: {path}")
        now = time.time()
        # 初始化文件内容
        self.data[path] = b'' 
        # 初始化文件元数据
        self.fd[path] = dict(
            st_mode=(S_IFREG | mode), 
            st_size=0, 
            st_ctime=now, 
            st_mtime=now, 
            st_atime=now, 
            st_nlink=1,
            st_uid=os.getuid(), 
            st_gid=os.getgid(),
        )
        # 返回一个文件描述符 (通常是 int)
        self.next_fd += 1
        return self.next_fd

    def unlink(self, path):
        """删除文件 (rm)。"""
        # logging.info(f"unlink: {path}")
        del self.fd[path]
        if path in self.data:
            del self.data[path]

    def truncate(self, path, length, fh=None):
        """截断文件到指定长度 (truncate)。"""
        # logging.info(f"truncate: {path}")
        self.data[path] = self.data[path][:length]
        self.fd[path]['st_size'] = length

    def rename(self, old, new, flags=0):
        """重命名文件或目录 (mv)。"""
        # logging.info(f"rename: {old} -> {new}")
        # 移动元数据
        self.fd[new] = self.fd.pop(old)
        # 移动数据
        if old in self.data:
            self.data[new] = self.data.pop(old)

    # --- 4. I/O 操作 (I/O Operations) ---
    
    def open(self, path, flags):
        """打开文件 (open())。"""
        # logging.info(f"open: {path}")
        self.next_fd += 1
        return self.next_fd

    def read(self, path, size, offset, fh):
        """读取文件内容 (read())。必选。"""
        # logging.info(f"read: {path}, offset: {offset}, size: {size}")
        if path not in self.data:
             raise FuseOSError(errno.ENOENT)
        # 返回切片后的文件内容
        return self.data[path][offset:offset + size]

    def write(self, path, data, offset, fh):
        """写入文件内容 (write())。"""
        # logging.info(f"write: {path}, offset: {offset}, data_len: {len(data)}")
        current_data = self.data.get(path, b'')
        
        # 写入操作可能发生在文件中间 (seek + write)
        before = current_data[:offset]
        after = current_data[offset + len(data):]
        
        self.data[path] = before + data + after
        
        # 更新文件大小和修改时间
        self.fd[path]['st_size'] = len(self.data[path])
        self.fd[path]['st_mtime'] = time.time()
        
        return len(data) # 必须返回实际写入的字节数

    def release(self, path, fh):
        """关闭文件 (close())。"""
        # logging.info(f"release: {path}")
        # 在内存FS中，这里通常不需要做任何事
        return 0

    def flush(self, path, fh):
        """刷新待写数据到内核缓存 (flush)。"""
        # 在内存FS中，数据已在内存，无需操作
        return 0

    def fsync(self, path, isdatasync, fh):
        """同步文件数据到存储设备 (fsync)。"""
        # 在内存FS中，无需操作
        return 0
    


# --- 主程序执行块 ---

if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser()
    parser.add_argument('mountpoint', help='挂载点目录 (例如: /mnt/myfs)')
    parser.add_argument('-f', '--foreground', action='store_true', help='在前台运行 (方便调试)')
    args = parser.parse_args()

    import os
    if not os.path.isdir(args.mountpoint):
         print(f"错误: 挂载点目录不存在: {args.mountpoint}")
         exit(1)

    print(f"--- 正在运行 FullMemoryFS，挂载点: {args.mountpoint} ---")
    
    # 运行 FUSE
    FUSE(FullMemoryFS(), args.mountpoint, foreground=args.foreground, allow_other=False)
    print("--- 文件系统已卸载 ---")
```

## 程序运行结果及分析

1. **挂载测试**：执行 `python3 fs.py /tmp/memfs`，文件系统成功挂载到指定目录。
2. **目录操作**：执行 `mkdir /tmp/memfs/test_dir`，通过 `ls` 命令可以看到目录创建成功，且 `st_nlink` 正确增加。
3. **文件读写**：
    * 执行 `echo "hello world" > /tmp/memfs/test.txt`，触发 `create` 和 `write` 操作。
    * 执行 `cat /tmp/memfs/test.txt`，触发 `open` 和 `read` 操作，正确输出 "hello world"。


4. **持久性验证**：卸载文件系统（`umount`）并重新挂载后，之前创建的文件消失，证实了其全内存运行的特性。

## 实验体会

### 遇到的问题及解决：

* **路径解析问题**：在 `readdir` 中，刚开始无法正确列出子目录。
* **解决**：通过字符串切片 `sub_path = name[len(path):]` 并过滤掉非直接子项，实现了对文件树的扁平化检索模拟。


* **写操作偏移量**：初次实现 `write` 时直接覆盖了原数据。
* **解决**：修正了逻辑，通过 `before + data + after` 的切片组合，支持了在文件任意位置（seek）进行改写。



### 收获与建议：

* **收获**：深刻理解了 VFS（虚拟文件系统）的工作原理。明白了当我们在终端输入 `ls` 或 `touch` 时，内核是如何通过 VFS 接口调用到具体底层实现的。
* **建议**：目前的实现是“扁平化”存储元数据（字典存储全路径），对于大规模文件系统效率较低。建议后续可以引入树状结构来管理 Inode，以优化高并发下的查询性能。
