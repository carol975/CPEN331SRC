

#include <vfs.h>
#include <types.h>
#include <vnode.h>
#include <kern/errno.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/limits.h>
#include <current.h>
#include <proc.h>
#include <uio.h>
#include <kern/iovec.h>
#include <filetable.h>
#include <limits.h>
#include <kern/fcntl.h>
#include <file_syscall.h>




int sys_open(const char *filename, int flags, mode_t mode){
    if(flags != O_RDONLY && flags != O_WRONLY && flags != O_RDWR){
        return EINVAL; // Invalid argument flags can only be one of the three 
    }
    
    char *fbuff;
    int result;
    
    if((fbuff = (char *)kmalloc(__PATH_MAX))== NULL){
        return ENOMEM;
    }
    
    //Copy filename (in usermode) to fbuff (in kernel mode))
    result = copyinstr((userptr_t)filename, fbuff, __PATH_MAX,NULL);
    
    //If copyinstr returns 1, then an error occured in copyinstr
    if(result){
        kfree(fbuff);
        return result;
    }
    
    //ft_add return file descriptor (int);
    result = ft_add(fbuff,flags,mode);
    
    //if ft_add returns 1 (false)
    if(result == -1){
        kfree(fbuff);
        return result;
    } 
    return result; //returns the file descriptor
    
}


int sys_write(int fd, char *buf, size_t buflen){
    
    struct iovec *fIOV = (struct iovec*)kmalloc(sizeof(struct iovec));
    struct uio *fUIO = (struct uio*)kmalloc(sizeof(struct uio));
    int result;
    //struct ftEntry file = curproc->filetable[fd];
    
    //TODO: aquire lock?
    
    //bad file number
    if((fd < 0) || (curproc->filetable[fd]) || (fd > FDS_MAX)) {
        //TODO: release lock?
        return EBADF;
    }
    
    fIOV->iov_ubase = (userptr_t)buf;    
    fIOV->iov_len = buflen;
    fUIO->uio_iov = fIOV;
    fUIO->uio_iovcnt = 1;
    fUIO->uio_offset = curproc->filetable[fd]->offset;
    fUIO->uio_resid = buflen;
    fUIO->uio_segflg = UIO_USERSPACE;
    fUIO->uio_rw = UIO_WRITE;
    fUIO->uio_space = curproc->p_addrspace;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
    
    result = VOP_WRITE(curproc->filetable[fd]->ft_vnode, fUIO);
    if(result){
        //TODO: release lock?
        return result;
    }
    
    curproc->filetable[fd]->offset = fUIO->uio_offset;
    
    
    return result;
    
}

/*
int sys_read(int fd , char *buf, size_t buflen){
 
    struct iovec *fIOV = (struct iovec*)kmalloc(sizeof(struct iovec));
    struct uio *fUIO = (struct uio*)kmalloc(sizeof(struct uio));
    int result;
    //struct ftEntry file = filetable[fd];
    
    if( fd< 0 || fd >= FDS_MAX || curproc->filetable[fd] == NULL){
        return EBADF; //return bad file number
    }
    
    lock_acquire(curproc->filetable[fd]-> ft_lock);
    if((curproc->filetable[fd]->mode) == O_WRONLY){
      lock_release(curproc->filetable[fd]->ft_lock);
      return EBADF;
    }
    
    
    fIOV->iov_ubase = (userptr_t)buf;    
    fIOV->iov_len = buflen;
    fUIO->uio_iov = fIOV;
    fUIO->uio_iovcnt = 1;
    fUIO->uio_offset = curproc->filetable[fd]->offset;
    fUIO->uio_resid = buflen;
    fUIO->uio_segflg = UIO_USERSPACE;
    fUIO->uio_rw = UIO_READ;
    fUIO->uio_space = curproc->p_addrspace; 
    
    result = VOP_READ(curproc->filetable[fd]->ft_vnode, fUIO);
    if(result){
        lock_release(curproc->filetable[fd]->ft_lock);
        return result;
    }
    
    lock_release(curproc->filetable[fd]->ft_lock);
    return result;

}   
*/
