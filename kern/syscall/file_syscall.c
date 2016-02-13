
#include <vfs.h>
#include <vnode.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/limits.h>

int sys_open(const char *filename, int flags){
    
    struct vnode *v;
    char *fbuff;
    int result;
    
    if(fbuff = (char *)kmalloc(__PATH_MAX))== NULL){
        return ENOMEM;
    }
    
    //Copy filename (in usermode) to fbuff (in kernel mode))
    result = copyinstr((userptr_t)filename, fbuff, __PATH_MAX,NULL);
    
    //If result is 1, then an error occured in copyinstr
    if(result){
        kfree(fbuff);
        return result;
    }
    //Add fbuff to file table
    
    result = vfs_open (fbuff, flags, 0, &v);
    
        
    
}
