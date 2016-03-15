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
	
	if(parent_tf == NULL){
		return EINVAL;
	}

    int err = 0;
    
    /*Create the child process*/
	 struct proc *child_proc = (struct proc *)kmalloc(sizeof(struct proc));
	 err = proc_fork(&child_proc);  //the file handle is copied automatically
	 
	 if(err){
		 kfree(child_proc);
		 return EMPROC;
	 }
	 
	 /*Copy the parent addresspace to child addresspace	 */
	err = as_copy(proc_getas(), &(child_proc->p_addrspace));
	 if(err){
		 return ENOMEM;
	 }
	 
	
	/* Child trapframe is a copy of the parent trapframe */
	struct trapframe *child_tf = (struct trapframe *)kmalloc(sizeof(struct trapframe));
	*child_tf = *parent_tf;
	
	/* Create the child thread and attach to child_proc*/

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
sys_execv(const char* program, char** uargs){
	
	if(program == NULL ||uargs == NULL ){
		return EFAULT;
	}
	
	int err;
	size_t actual;
	
	// Copy the program from user space to kernel space
	char* k_program = (char*)kmalloc(sizeof(char) * PATH_MAX);
	
	err = copyinstr((const_userptr_t)program,k_program,PATH_MAX,&actual);
	
	if(err){
		kfree(k_program);
		return EFAULT;
	}
	
	if(actual){
		//empty program name
		kfree(k_program);
		return EINVAL;
	}
	
	/*char **k_args = (char**)kmalloc(sizeof(char**));
	err = copyin((const_userptr_t)uargs, k_args,sizeof(char*));
	
	if(err){
		kfree(k_program);
		kfree(k_args);
		return EFAULT;
	}
	
	*/
	// Copy the user arguments into kernel arguments
	int nargs = 0;
	
	while(uargs[nargs] != NULL){
		++nargs;
	}
	
	char ** k_args = (char **)kmalloc(sizeof(char) * nargs);

	int i;
	
	for(i = 0; i < nargs, ++i){
		err = copyinstr((const_userptr_t)uargs[i], k_args[i],PATH_MAX,&actual);
		if(err){
			kfree(k_program);
			kfree(k_args);
			return EFAULT;
		}
	}
	
	++i;
	args[i] = NULL;
	
	
	
	//========================================================================
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	
	/*Open the file */
	err = vfs_open((char *)program, O_RDONLY,0,&v);
	if(err){
		kfree(k_program);
		kfree(k_args);
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
		kfree(k_program);
		kfree(k_args);
		vfs_close(v);
		return ENOMEM;
	}
	
	// Switch to the newly created addresspace, that is, to completely abandon the addrspce it inherited from its parent
	
	proc_setas(as);
	
	
	/* Load thee executable. */
	err = load_elf(v, &entrypoint);
	if(err){
		/* p_addrspace will be destroyed when curproc is destroyed */
		kfree(k_program);
		kfree(k_args);
		vfs_close(v);
		return err; 
	}
	
	/*Done with the file now. */
	vfs_close(v);
	
	/* Define the user stack in the address space */
	err = as_define_stack(as, &stackptr);
	if(err){
		kfree(k_program);
		kfree(k_args);
		return err;
	}
	
	
	
	//==================================================================
	//copy out the arguments
	int arg_len;
	int j = 0;
	while(k_args[j] != NULL){
		//get the length for 1 string of all strings in k_args
		arg_len = strlen(k_args[j]) + 1; // extra space for null terminator
		
		//one string
		char* arg_part;
		
		//allocate memory for the size of the current string in k_args
		arg_part = kmalloc(sizeof(arg_len));
		arg_part = kstrdup(k_args[j]);
		
		int index;
		
		//extract one string from k_args
		for(index = 0; index < arg_len; ++index){
			arg_part[index] = k_args[j][index];
		}
		
		//decrease the stack pointer by one memory block, i.e. move to the next string
		stackptr -= arg_len;
		
		err = copyout((const void *)arg_part, (userptr_t)stackptr, (size_t)arg_len);
		if(err){
			kfree(k_program);
			kfree(k_args);
			kfree(arg_part);
			return err;
		}
		
		kfree(arg_part);
		
		k_args[j] = (char *)stackptr;
		
		for(in)
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
    
    //invalid status pointer
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
    
    //wait on exit if child not exited yet
    if(pidTable[pid]->exited == false){
        P(pidTable[pid]->sem_wait);
    }
    
    //copy exit status of chile to parent
    int result = copyout((void *) &(pidTable[pid]->exitCode), (userptr_t)status, sizeof(int));
    if(result){
        return EFAULT;
    }
    
    //lock the process to wait for exit
    V(pidTable[pid]->sem_exit);
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

    
    struct proc *current_proc = curproc;  //save the current proc for later use
    pid_t pid = curproc->pid;
    
    //set the child exit status and flag
    pidTable[pid]->exited = true;
    pidTable[pid]->exitCode = _MKWAIT_EXIT(exitcode);
    
    //signal parent to release sem
    V(pidTable[pid]->sem_wait);     
      
    //wait on the parent to signal it's time to exit
    P(pidTable[pid]->sem_exit);
    
    //clean up the child
    proc_remthread(curthread);
    kfree(pidTable[pid]);
    pidTable[pid] = NULL;
    proc_destroy(current_proc);
    proc_addthread(kproc, curthread);
    thread_exit();
}





