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
	 struct proc *child_proc = (proc *)kmalloc(sizeof(struct proc));
	 err = proc_fork(&child_proc);  //the file handle is copied automatically
	 
	 /*Copy the parent addresspace to child addresspace	 */
	as_copy(proc_getas(), child_proc->p_addrspace);
	 if(as == ENOMEM){
		 return ENOMEM;
	 }
	 
	 
	 /*Save a copy of the parent addresspace*/
	//struct addrspace * parent_addrspace = (addrspace*)kmalloc(sizeof(addrspace));
	
	 //is switching addrspace automatically switching the process?????? else how????
	//parent_addrspace = proc_setas(child_proc->addrspace);

	

	/* Child trapframe is a copy of the parent trapframe */
	struct trapframe *child_tf = (trapframe *)kmalloc(sizeof(trapframe));
	*child_tf = *parent_tf;
	
	
	/* Create the child thread and attach to child_proc*/
	//Should child_proc be replaced with NULL? since the process is switched
	err = thread_fork("Child Thread", child_proc, &enter_forked_process,(void *)child_tf,0);
	if(err){
		kprintf("child thread_fork failed: %s\n",strerror(err));
		proc_destroy(child_proc);
		return err;
	}
	
	//99999 is a dummy value for child pid 
	*retval = 99999;
	 return err;

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



int sys_execv(const char* program, char** args, int *retval){
	//MORE STUFF BEFORE
	
	/* open the executable, create a new address space and load the elf into it */
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptrl
	int err;
	/*Open the file */
	err = vfs_open(program, O_RDONLY,0,&v);
	if(err){= 
		return err;
	}
	
	/* since execv is called after fork, we should be in the new process */
	// since file table for child is already copied over from the parent, no need to
	// create a new 
	as = as_create();
	if(as == NULL){
		vfs_clos(v);
		return ENOMEM;
	}
	
	// Switch to the newly created addresspace, that is, to completely abandon the addrspce it inherited from its parent
	
	proc_setas(as);
	as_activate();
	
	/* Load thee executable. */
	err = load_elf(v, &entrypoint);
	if(err){
		/* p_addrspace will be destroyed when curproc is destroyed */
		vfs_close(v);
		return err; 
	}
	
	/* Go back to user mode */
	enter_new_process(0/*argc*/,NULL/*userspace addr of argv*/, NULL ./*userspace addr of environment */, stackptr, entrypoint);
	
	/*enter_new_process should not return */
	panic("enter_new_process returned\n");
	return EINVAL;
	
}
