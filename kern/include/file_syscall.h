#ifndef _FILE_SYSCALL_H_
#define _FILE_SYSCALL_H_

#include <vfs.h>
#include <vnode.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/limits.h>
#include <current.h>
#include <proc.h>
#include <uio.h>
#include <kern/iovec.h>
#include <filetable.h>
#include <limits.h>

int sys_open(const char *filename, int flags, mode_t mode);

int sys_read(int fd, char *buf, size_t buflen);

int sys_write(int fd, char *buf, size_t buflen);

#endif
