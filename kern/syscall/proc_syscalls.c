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
sys_fork(struct trapframe *parent_tf, int *retval){

 int err = 0;
	 
	 /*Create the child process*/
	 struct proc *child_proc = (proc *)kmalloc(sizeof(proc));
	 err = proc_fork(&child_proc);  //the file handle is copied automatically
	 
	 /*Copy the parent addresspace to child addresspace	 */
	 //NOTE: consider child_as = parent_as_tf+4
	as_copy(curproc->p_addrspace, child_proc->p_addrspace);
	 if(as == ENOMEM){
		 return ENOMEM;
	 }
	 
	 /*Modify child_addr */

	 //TODO
	 
	 
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
	err = thread_fork("Child Thread", child_proc,child_fork_entry,NULL,0);
	if(err){
		kprintf("child thread_fork failed: %s\n",strerror(err));
		proc_destroy(child_proc);
		return err;
	}
	//
	
	 return 0;


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
