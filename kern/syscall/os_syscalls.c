/*
 * This is the source file for fork, execv, waitpid, getpid, and _exit syscalls
 */
 
#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/unistd.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <openfile.h>
#include <filetable.h>
#include <syscall.h>
#include <test.h>

#include <thread.h>

static void child_forkentry(void *p, unsigned long arg){
	//TODO
	(void)p;
	(void)arg;
}
int sys_fork(trapframe *parent_tf){
	 
	 int err = 0;
	 
	 /*Create the child process*/
	 struct proc *child_proc = (proc *)kmalloc(sizeof(proc));
	 err = proc_fork(&child_proc);  //the file handle is copied automatically
	 
	 /*Copy the parent addresspace to child addresspace	 */
	 //NOTE: consider child_as = parent_as_tf+4
	as_copy(curproc->p_addrspace, child_proc->addrspace);
	 if(as == ENOMEM){
		 return ENOMEM;
	 }
	 
	 //switch to the child process here?????????
	
	/* Create the child thread */
	struct trapframe *child_tf = (trapframe *)kmalloc(sizeof(trapframe));
	child_tf = parent_tf;
	
	err = thread_fork("Child Thread", child_proc,child_fork_entry,NULL,0);
	if(err){
		kprintf("child thread_fork failed: %s\n",strerror(err));
		proc_destroy(child_proc);
		return err;
	}
	
	 return 0;
 }