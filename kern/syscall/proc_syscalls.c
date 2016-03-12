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
pid_t 
sys_fork(struct trapframe *parent_tf, int *retval){

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
	
	struct processID *child_info = (struct processID *)kmalloc(sizeof(struct processID));
	
	child_info->pid = child_proc->pid;
	child_info->ppid = curproc->pid;
	child_info->exited = 0;
	child_info->exitCode = 0;
	
	pidTable[child_proc->pid] = child_info;
	//99999 is a dummy value for child pid
	//for parent fork() returns the dummy child pid
	*retval = child_proc->pid;
	return 0;
}


/* execute a program */
/*
int 
sys_execv(const char* program, char** args, int *retval){

	 return 0;


}
*/

/* wait for a process to exit */
/*
pid_t 
sys_waitpid(pid_t pid, int* status, int option, int *retval){

}
*/

/* get process id */
pid_t 
sys_getpid(int *retval){

    *retval = curproc->pid;
    return 0;
}

/* terminate a process */
/*
void 
sys__exit(int exitcode){

}
*/




