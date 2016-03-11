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
#include <thread.h>
#include <addrspace.h>
#include <mips/trapframe.h>

/* copy the current process */
pid_t sys_fork(struct trapframe *parent_tf, int *retval){

 int err = 0;
	 
	 /*Create the child process*/
	 struct proc *child_proc = (struct proc *)kmalloc(sizeof(struct proc));
	 err = proc_fork(&child_proc);  //the file handle is copied automatically
	 //check//
	 
	 /*Copy the parent addresspace to child addresspace	 */
	err = as_copy(proc_getas(), &(child_proc->p_addrspace));
	 if(err == ENOMEM){
		 return ENOMEM;
	 }
	
	/* Child trapframe is a copy of the parent trapframe */
	struct trapframe *child_tf = (struct trapframe *)kmalloc(sizeof(struct trapframe));
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
	//for parent fork() returns the dummy child pid
	*retval = 999;
	 return 0;

}


pid_t sys_getpid(int *retval){
		*retval = curproc->pid;
		return 0;
}

int sys_execv(const char* program, char** args, int *retval){
	
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int err;
	
	/* open the executable, create a new address space and load the elf into it */
	
	/*Open the file */
	err = vfs_open(program, O_RDONLY,0,&v);
	if(err){
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
