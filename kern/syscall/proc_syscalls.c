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
#include <kern/wait.h>

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
	child_info->exited = false;
	child_info->exitCode = 0;
	child_info->p_lk = lock_create("pid_lock");
	if(child_info->p_lk == NULL){
	    return 0;
	}
	child_info->p_cv = cv_create("pid_cv");
	if(child_info->p_cv == NULL){
	    return 0;
	}
	
	
	pidTable[child_proc->pid] = child_info;
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
pid_t 
sys_waitpid(pid_t pid, int* status, int option, int *retval){
    
    
    //out of the bounds of valid pid
    if(pid > PID_MAX || pid < PID_MIN){
        return ESRCH;
    }
    
    //have same pid as current process
    if(pid == curproc->pid){
        return ECHILD;
    }
    
    //invalide status pointer
    if(status == NULL){
        return EFAULT;
    }
    
    //option should be 0
    if(option != 0){
        return EINVAL;
    }
    
    //check if pid matches and entry in the process table
    pid_t id;
    for(id = PID_MIN; id < PID_MAX; id++){
        if(pidTable[id]->pid == pid){
            break;
        }
    }
    
    //if no entry was found with the index
    if(pid == PID_MAX){
        return ESRCH;
    }
    
    
    //wait for the process to exit
    lock_acquire(pidTable[id]->p_lk);
    
    while(pidTable[id]->exited != true){
        cv_wait(pidTable[id]->p_cv, pidTable[id]->p_lk);
    }
    lock_release(pidTable[id]->p_lk);
    
    int result = copyout((void *) &(pidTable[id]->exitCode), (userptr_t)status, sizeof(int));
    if(result){
        return result;
    }
    
    *retval = pid;
    return 0;
}


/* get process id */
pid_t 
sys_getpid(int *retval){

    *retval = curproc->pid;
    return 0;
}

/* terminate a process */
void 
sys__exit(int exitcode){

    //struct proc *current_proc = curproc;
    pid_t pid = curproc->pid;
    struct proc *current = curproc;
    
    pidTable[pid]->exited = true;
    pidTable[pid]->exitCode = _MKWAIT_EXIT(exitcode);
    
    proc_remthread(curthread);
    
    proc_destroy(current);
    
    kfree(pidTable[pid]);
    pidTable[pid] = NULL;
    
    thread_exit();
}





