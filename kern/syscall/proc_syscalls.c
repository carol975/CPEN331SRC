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

	pidTable[child_proc->pid]->ppid = curproc->pid;
	*retval = child_proc->pid;
	return 0;
}


/* execute a program */

int 
sys_execv(const char* program, char** args){

	 struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int err;
	
	if(args == NULL){
	    return EFAULT;
	}
	/* open the executable, create a new address space and load the elf into it */
	
	/*Open the file */
	err = vfs_open((char *)program, O_RDONLY,0,&v);
	if(err){
		return err;
	}
	
	/* since execv is called after fork, we should be in the new process */
	// since file table for child is already copied over from the parent, no need to
	// create a new file table
	
	// destroy the child_proc addrspace
	if(curproc->p_addrspace != NULL){
		as_destroy(curproc->p_addrspace);
		curproc->p_addrspace = NULL;
	}
	
	// create a new addrspace for the child proc
	as = as_create();
	if(as == NULL){
		vfs_close(v);
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
	
	/*Done with the file now. */
	vfs_close(v);
	
	/* Define the user stack in the address space */
	err = as_define_stack(as, &stackptr);
	if(err){
		return err;
	}
	
	/* Go back to user mode */
	/* Note: argc should be tf_a0, argv should be tf_a1l
	*/
	enter_new_process(0/*argc*/,NULL/*userspace addr of argv*/, NULL /*userspace addr of environment */, stackptr, entrypoint);
	
	/*enter_new_process should not return */
	panic("enter_new_process returned\n");
	return EINVAL;


}


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
    
    //pid_t parent_pid = pidTable[pid]->ppid;
    /*
    //wait for the process to exit
    lock_acquire(pidTable[pid]->p_lk);
    
    // || pidTable[parent_pid]->exited != true
    while(pidTable[pid]->exited != true){
        cv_wait(pidTable[pid]->p_cv, pidTable[pid]->p_lk);
    }
    lock_release(pidTable[pid]->p_lk);
    */
    P(pidTable[pid]->sem_wait);
    int result = copyout((void *) &(pidTable[pid]->exitCode), (userptr_t)status, sizeof(int));
    if(result){
        return result;
    }
    V(pidTable[pid]->sem_exit);
    //proc_destroy(pidTable[pid]);
    
    kfree(pidTable[pid]);
    pidTable[pid] = NULL;
    
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

    
    struct proc *current_proc = curproc;
    pid_t pid = curproc->pid;
    
    if(pidTable[pid]->exited == false){
        pidTable[pid]->exited = true;
        pidTable[pid]->exitCode = _MKWAIT_EXIT(exitcode);
        V(pidTable[pid]->sem_wait);
        
        P(pidTable[pid]->sem_exit);
        proc_remthread(curthread);
        kfree(pidTable[pid]);
        pidTable[pid] = NULL;
        
        
        proc_destroy(current_proc);
    }
    proc_addthread(kproc, curthread);
    thread_exit();
    
}





