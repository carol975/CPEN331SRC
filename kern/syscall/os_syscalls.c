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
	 if(child_proc == NULL){
		 return err;
	 }
	 
	 /*Copy the parent addresspace to child addresspace	 */
	 //NOTE: consider child_as = parent_as_tf+4
	 //proc_setas() in as_copy?
	as_copy(curproc->p_addrspace, child_proc->p_addrspace);
	 if(as == ENOMEM){
		 return ENOMEM;
	 }
	 
	 /*Modify child_addr */

	 //TODO copy file table to an array  with array_create()
	 
	 /*Save a copy of the parent addresspace*/
	struct addrspace * parent_addrspace = (addrspace*)kmalloc(sizeof(addrspace));
	 //switch to the child process here?????????
	 //is switching addrspace automatically switching the process?????? else how????
	parent_addrspace = proc_setas(child_proc->addrspace);
	as_activate();
	
	/* Create the child thread and attach to child_proc*/
	struct trapframe *child_tf = (trapframe *)kmalloc(sizeof(trapframe));
	child_tf = parent_tf;
	
	//Should child_proc be replaced with NULL? since the process is switched
	err = thread_fork("Child Thread", child_proc,&child_fork_entry,NULL,0);
	if(err){
		kprintf("child thread_fork failed: %s\n",strerror(err));
		proc_destroy(child_proc);
		return err;
	}
	//
	
	 return 0;
 }