/*
 * Process-related system call implementations
 */
 
#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <copyinout.h>
#include <vfs.h>
#include <vnode.h>
#include <openfile.h>
#include <filetable.h>
#include <syscall.h>

/* copy the current process */
pid_t 
sys_fork(int *retval){


}

/* execute a program */
int 
sys_execv(const char* program, char** args, int *retval){

}

/* wait for a process to exit */
pid_t 
sys_waitpid(pid_t pid, int* status, int option, int *retval){

}

/* get process id */
pid_t 
sys_getpid(int *retval){

}

/* terminate a process */
void 
sys__exit(int exitcode){

}
