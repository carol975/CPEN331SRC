
#include <vfs.h>
#include <vnode.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/limits.h>
#include <proc.h>


int sys_open(const char *filename, int flags, mode_t mode){
    if(flag != O_RDONLY && flag != O_WRONLY && flag != O_RDWR){
        return EINVAL; // Invalid argument flags can only be one of the three 
    }
    
    struct vnode *v;
    char *fbuff;
    int result;
    
    if(fbuff = (char *)kmalloc(__PATH_MAX))== NULL){
        return ENOMEM;
    }
    
    //Copy filename (in usermode) to fbuff (in kernel mode))
    result = copyinstr((userptr_t)filename, fbuff, __PATH_MAX,NULL);
    
    //If copyinstr returns 1, then an error occured in copyinstr
    if(result){
        kfree(fbuff);
        return result;
    }
    //TODO:Add fbuff to file table
    
    result = vfs_open (fbuff, flags, mode, &v);
    
    //if vfs_open returns 1 (false)
    if(result){
        kfree(kbuff);
        
        //TODO: free the the pointer of the opened file in filetable
        
        return result;
    }
    
    //TODO: Set all fields of for this file in filetable
    
    return 0;
    
}

//fd: file descriptor
int sys_read(int fd , void *buf, size_t buflen){
 
    if( fd < 0 || fd >= FDS_MAX || fd_table[fd] == NULL){
        return EBADF; //return bad file number
    }
    
    
    
    
    
    
    
    


