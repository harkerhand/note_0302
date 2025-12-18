# 操作系统实验报告：Linux进程管理及其扩展

## 基本信息

* **姓名**：巩皓锴
* **学号**：09023321
* **日期**：2025年12月17日


## 实验内容

1. 分析Linux内核源代码中关于进程控制块（`task_struct`）及进程组织的数据结构。
2. 实现系统调用 `hide(pid_t pid, int on)`：根据PID隐藏或恢复指定进程。
3. 实现系统调用 `hide_user_processes(uid_t uid, char *binname)`：隐藏指定用户的所有进程或特定映像名的进程。
4. 在 `/proc` 目录下实现 `hidden` 控制文件，作为隐藏功能的全局开关。
5. 在 `/proc` 目录下实现 `hidden_process` 文件，实时列出当前所有被隐藏的PID。



## 实验目的

1. 深入理解Linux内核中进程的表示方法（`task_struct`）及进程链表的组织方式。
2. 掌握Linux系统调用的添加流程及内核编译方法。
3. 理解 `/proc` 虚拟文件系统的工作机制及内核与用户态的交互。
4. 掌握内核态链表操作、并发控制（Mutex）及权限检查（Cred）的安全实现。



## 设计思路

* **核心逻辑**：Linux中 `ps` 和 `top` 命令通过读取 `/proc/[pid]` 目录获取进程信息。因此，隐藏进程的核心在于修改 `fs/proc` 的目录遍历逻辑。当 `proc` 尝试遍历进程列表时，我们插入一个判定函数 `is_hidden()`。
* **存储机制**：在内存中维护三张链表，分别记录“按PID隐藏”、“按UID隐藏”、“按UID+名称隐藏”的规则。
* **系统调用**：新增两个系统调用，通过 `copy_from_user` 获取参数，并对调用者进行 `ROOT` 权限校验，安全地修改内核链表。
* **全局开关**：利用 `/proc/hidden` 对应的全局变量 `hidden_flag` 充当总闸。



## 主要数据结构及其说明

本实验定义了以下关键数据结构来管理隐藏状态：

```c
struct hidden_pid {
	pid_t pid;
	struct list_head list;
};

struct hidden_uid {
	uid_t uid;
	struct list_head list;
};

struct hidden_uid_bin {
	uid_t uid;
	char comm[TASK_COMM_LEN];
	struct list_head list;
};

static LIST_HEAD(pid_hidden_list);
static LIST_HEAD(uid_hidden_list);
static LIST_HEAD(uid_bin_hidden_list);
static DEFINE_MUTEX(hidden_lock);
```

## 源程序

以下为内核扩展模块的核心实现逻辑：

```c
// kernel/hide.c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/hide.h>

/* global hidden switch */
int hidden_flag = 1;

// 读hidden文件
static ssize_t hidden_read(struct file *file, char __user *buf, size_t count,
			   loff_t *ppos)
{
	char tmp[8];
	int len = snprintf(tmp, sizeof(tmp), "%d\n", hidden_flag);
	return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

// 写hidden文件
static ssize_t hidden_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	char kbuf[8];

	if (count > sizeof(kbuf) - 1)
		return -EINVAL;

	if (copy_from_user(kbuf, buf, count))
		return -EFAULT;

	kbuf[count] = '\0';

	if (kbuf[0] == '0')
		hidden_flag = 0;
	else
		hidden_flag = 1;

	return count;
}

static const struct proc_ops hidden_fops = {
	.proc_read = hidden_read,
	.proc_write = hidden_write,
};

// 读hidden_process文件
static ssize_t hidden_process_read(struct file *file, char __user *buf,
				   size_t count, loff_t *ppos)
{
	struct task_struct *task;
	char *kbuf;
	size_t len = 0, bufsize = 4096;

	/* 分配临时 buffer */
	kbuf = kzalloc(bufsize, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	/* 遍历进程列表 */
	rcu_read_lock();
	for_each_process(task) {
		if (is_hidden(task)) {
			len += scnprintf(kbuf + len, bufsize - len, "%d ",
					 task->pid);
			if (len >= bufsize - 16)
				break;
		}
	}
	rcu_read_unlock();

	/* 返回数据 */
	ssize_t ret = simple_read_from_buffer(buf, count, ppos, kbuf, len);

	kfree(kbuf);
	return ret;
}

// 写hidden_process文件
static const struct proc_ops hidden_process_fops = {
	.proc_read = hidden_process_read,
};

struct hidden_pid {
	pid_t pid;
	struct list_head list;
};

struct hidden_uid {
	uid_t uid;
	struct list_head list;
};

struct hidden_uid_bin {
	uid_t uid;
	char comm[TASK_COMM_LEN];
	struct list_head list;
};

static LIST_HEAD(pid_hidden_list);
static LIST_HEAD(uid_hidden_list);
static LIST_HEAD(uid_bin_hidden_list);
static DEFINE_MUTEX(hidden_lock);

bool is_hidden(struct task_struct *task)
{
	if (!hidden_flag)
		return false;

	struct hidden_pid *hp;
	struct hidden_uid *hu;
	struct hidden_uid_bin *hub;
	bool hidden = false;
	uid_t t_uid = (uid_t)-1;
	const struct cred *tcred;

	if (!task)
		return false;

	tcred = get_task_cred(task);

	if (tcred) {
		t_uid = tcred->uid.val;
		put_cred(tcred);
	} else {
		/* 无法取得 cred，保守起见不隐藏 */
		return false;
	}

	mutex_lock(&hidden_lock);

	// 按 PID 隐藏
	list_for_each_entry(hp, &pid_hidden_list, list) {
		if (hp->pid == task->pid) {
			hidden = true;
			goto out;
		}
	}

	// 按 UID 隐藏
	list_for_each_entry(hu, &uid_hidden_list, list) {
		if (hu->uid == t_uid) {
			hidden = true;
			goto out;
		}
	}

	// 按 UID + 进程名隐藏
	list_for_each_entry(hub, &uid_bin_hidden_list, list) {
		if (hub->uid == t_uid && strcmp(hub->comm, task->comm) == 0) {
			hidden = true;
			goto out;
		}
	}
out:
	mutex_unlock(&hidden_lock);
	return hidden;
}

int hide_pid(pid_t pid, int on)
{
	struct hidden_pid *node, *tmp;

	mutex_lock(&hidden_lock);

	if (on) {
		// 已经存在就不用再添加
		list_for_each_entry(node, &pid_hidden_list, list) {
			if (node->pid == pid) {
				mutex_unlock(&hidden_lock);
				return 0;
			}
		}

		node = kmalloc(sizeof(*node), GFP_KERNEL);
		if (!node) {
			mutex_unlock(&hidden_lock);
			return -ENOMEM;
		}
		node->pid = pid;
		list_add(&node->list, &pid_hidden_list);
	} else {
		// 取消隐藏：从链表删除
		list_for_each_entry_safe(node, tmp, &pid_hidden_list, list) {
			if (node->pid == pid) {
				list_del(&node->list);
				kfree(node);
				break;
			}
		}
	}

	mutex_unlock(&hidden_lock);
	return 0;
}

int hide_uid(uid_t uid, int on)
{
	struct hidden_uid *node, *tmp;
	mutex_lock(&hidden_lock);
	if (on) {
		list_for_each_entry(node, &uid_hidden_list, list) {
			if (node->uid == uid) {
				mutex_unlock(&hidden_lock);
				return 0;
			}
		}
		node = kmalloc(sizeof(*node), GFP_KERNEL);
		if (!node) {
			mutex_unlock(&hidden_lock);
			return -ENOMEM;
		}
		node->uid = uid;
		list_add(&node->list, &uid_hidden_list);
	} else {
		list_for_each_entry_safe(node, tmp, &uid_hidden_list, list) {
			if (node->uid == uid) {
				list_del(&node->list);
				kfree(node);
				break;
			}
		}
	}
	mutex_unlock(&hidden_lock);
	return 0;
}

int hide_uid_bin(uid_t uid, const char *comm, int on)
{
	struct hidden_uid_bin *node, *tmp;
	mutex_lock(&hidden_lock);

	if (on) {
		list_for_each_entry(node, &uid_bin_hidden_list, list) {
			if (node->uid == uid && strcmp(node->comm, comm) == 0) {
				mutex_unlock(&hidden_lock);
				return 0;
			}
		}

		node = kmalloc(sizeof(*node), GFP_KERNEL);
		if (!node) {
			mutex_unlock(&hidden_lock);
			return -ENOMEM;
		}

		node->uid = uid;
		strncpy(node->comm, comm, TASK_COMM_LEN);
		list_add(&node->list, &uid_bin_hidden_list);

	} else {
		list_for_each_entry_safe(node, tmp, &uid_bin_hidden_list,
					 list) {
			if (node->uid == uid && strcmp(node->comm, comm) == 0) {
				list_del(&node->list);
				kfree(node);
				break;
			}
		}
	}

	mutex_unlock(&hidden_lock);
	return 0;
}

SYSCALL_DEFINE2(hide, pid_t, pid, int, on)
{
	if (!uid_eq(current_euid(), GLOBAL_ROOT_UID))
		return -EPERM;

	return hide_pid(pid, on);
}
SYSCALL_DEFINE2(hide_user_processes, uid_t, uid, char __user *, binname)
{
	char kname[TASK_COMM_LEN] = { 0 };

	if (!uid_eq(current_euid(), GLOBAL_ROOT_UID))
		return -EPERM;

	if (binname) {
		if (strncpy_from_user(kname, binname, sizeof(kname)) < 0)
			return -EFAULT;

		return hide_uid_bin(uid, kname, 1);
	}
	return hide_uid(uid, 1);
}

static int __init hide_init(void)
{
	proc_create("hidden", 0666, NULL, &hidden_fops);
	proc_create("hidden_process", 0444, NULL, &hidden_process_fops);
	printk(KERN_INFO "hide module loaded\n");
	return 0;
}

static void __exit hide_exit(void)
{
	remove_proc_entry("hidden", NULL);
	remove_proc_entry("hidden_process", NULL);
	printk(KERN_INFO "hide module unloaded\n");
}

module_init(hide_init);
module_exit(hide_exit);
MODULE_LICENSE("GPL");

```
```c
// include/linux/hide.h
#ifndef _LINUX_HIDE_H
#define _LINUX_HIDE_H

#include <linux/types.h>

bool is_hidden(struct task_struct *task);

int hide_pid(pid_t pid, int on);

int hide_uid(uid_t uid, int on);

int hide_uid_bin(uid_t uid, const char *comm, int on);

#endif
```
```c
// fs/proc/base.c
// ...
struct dentry *proc_pid_lookup(struct dentry *dentry, unsigned int flags)
{
	struct task_struct *task;
	unsigned tgid;
	struct proc_fs_info *fs_info;
	struct pid_namespace *ns;
	struct dentry *result = ERR_PTR(-ENOENT);

	tgid = name_to_int(&dentry->d_name);
	if (tgid == ~0U)
		goto out;

	fs_info = proc_sb_info(dentry->d_sb);
	ns = fs_info->pid_ns;
	rcu_read_lock();
	task = find_task_by_pid_ns(tgid, ns);
	if (is_hidden(task)) // 此处添加
		task = NULL;
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		goto out;

	/* Limit procfs to only ptraceable tasks */
	if (fs_info->hide_pid == HIDEPID_NOT_PTRACEABLE) {
		if (!has_pid_permissions(fs_info, task, HIDEPID_NO_ACCESS))
			goto out_put_task;
	}

	result = proc_pid_instantiate(dentry, task, NULL);
out_put_task:
	put_task_struct(task);
out:
	return result;
}
// ...
int proc_pid_readdir(struct file *file, struct dir_context *ctx)
{
	struct tgid_iter iter;
	struct proc_fs_info *fs_info = proc_sb_info(file_inode(file)->i_sb);
	struct pid_namespace *ns = proc_pid_ns(file_inode(file)->i_sb);
	loff_t pos = ctx->pos;

	if (pos >= PID_MAX_LIMIT + TGID_OFFSET)
		return 0;

	if (pos == TGID_OFFSET - 2) {
		struct inode *inode = d_inode(fs_info->proc_self);
		if (!dir_emit(ctx, "self", 4, inode->i_ino, DT_LNK))
			return 0;
		ctx->pos = pos = pos + 1;
	}
	if (pos == TGID_OFFSET - 1) {
		struct inode *inode = d_inode(fs_info->proc_thread_self);
		if (!dir_emit(ctx, "thread-self", 11, inode->i_ino, DT_LNK))
			return 0;
		ctx->pos = pos = pos + 1;
	}
	iter.tgid = pos - TGID_OFFSET;
	iter.task = NULL;
	for (iter = next_tgid(ns, iter);
	     iter.task;
	     iter.tgid += 1, iter = next_tgid(ns, iter)) {
		char name[10 + 1];
		unsigned int len;

		cond_resched();
		if (is_hidden(iter.task)) continue; // 此处添加
		if (!has_pid_permissions(fs_info, iter.task, HIDEPID_INVISIBLE))
			continue;

		len = snprintf(name, sizeof(name), "%u", iter.tgid);
		ctx->pos = iter.tgid + TGID_OFFSET;
		if (!proc_fill_cache(file, ctx, name, len,
				     proc_pid_instantiate, iter.task, NULL)) {
			put_task_struct(iter.task);
			return 0;
		}
	}
	ctx->pos = PID_MAX_LIMIT + TGID_OFFSET;
	return 0;
}
// ...
```



## 程序运行结果及分析

### 实验步骤与测试：

1. **编译与加载**：编译内核或加载该模块。
2. **验证 hide(pid, 1)**：启动一个 `top` 进程，记录其 PID 为 1234。调用 `hide(1234, 1)` 后，再次输入 `ps -ef`，发现进程 1234 已不再显示。
3. **验证全局开关**：执行 `echo 0 > /proc/hidden`。此时即便 PID 1234 在隐藏名单中，`ps` 依然能看到它，证明全局开关生效。
4. **验证隐藏用户进程**：调用 `hide_user_processes(1000, NULL)`，切换到该普通用户，其所有进程在 `top` 中消失。
5. **读取 hidden_process**：执行 `cat /proc/hidden_process`，输出结果为 `1234 ...`，准确列出了当前处于“待隐藏”状态的 PID 列表。

### 结果分析：

该设计通过在 `fs/proc/base.c` 的 `proc_pid_readdir` 和 `proc_pid_lookup` 函数中引入 `is_hidden` 检查，从虚拟文件系统的源头切断了用户态探测进程的路径，达到了深度隐藏的目的，且不影响进程的正常运行（如 CPU 调度）。



## 实验体会

### 遇到的问题及解决：

* **并发冲突**：最初未加 `mutex` 锁，当多个进程同时修改隐藏列表时出现了内核写错误。通过引入 `DEFINE_MUTEX` 解决了同步问题。
* **UID 获取安全**：直接访问 `task->cred` 不安全。学习后改用 `get_task_cred()` 并配合 `put_cred()`，遵循了内核的引用计数机制。

### 收获与建议：

* **收获**：通过本次实验，我深刻理解了“一切皆文件”的思想在 Linux 进程管理中的体现。学会了如何编写健壮的内核代码，特别是在处理用户态数据拷贝时的边界检查。
* **建议**：今后的实验可以尝试结合内核 Rootkit 技术，探索如何不仅在 `/proc` 中隐藏，甚至在内核调度器层面使进程“不可见”，以加深对内核安全性的认识。

