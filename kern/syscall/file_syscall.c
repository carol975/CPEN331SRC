

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




int sys_open(const char *filename, int flags, mode_t mode, int *retval){
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
    
    *retval = result;
    return 0; //returns the file descriptor
    
}


int sys_write(int fd, char *buf, size_t buflen, int *retval){
    
    struct iovec *fIOV = (struct iovec*)kmalloc(sizeof(struct iovec));
    struct uio *fUIO = (struct uio*)kmalloc(sizeof(struct uio));
    int result;
    char *fbuf;
    
    
    lock_acquire(curproc->filetable[fd]->ft_lock);
    
    //bad file number
    if((fd < 0) || (curproc->filetable[fd]) || (fd > FDS_MAX)) {
        lock_release(curproc->filetable[fd]->ft_lock);
        return EBADF;
    }
    
    if((curproc->filetable[fd]->mode) == O_WRONLY){
      lock_release(curproc->filetable[fd]->ft_lock);
      return EBADF; //bad file number
    }
    
    
    fbuf = kmalloc(sizeof(*buf)*buflen);
    if(fbuf == NULL){
        return EINVAL;
    }
    
    lock_acquire(curproc->filetable[fd]->ft_lock);
    result = copyinstr((userptr_t)buf, fbuf, buflen, NULL);
    if(result){
        kfree(fbuf);
        lock_release(curproc->filetable[fd]->ft_lock);
        return result;
    }
    
    fIOV->iov_ubase = (userptr_t)fbuf;    
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
        kfree(fbuf);
        lock_release(curproc->filetable[fd]->ft_lock);
        return result;
    }
    
    curproc->filetable[fd]->offset = fUIO->uio_offset;
    *retval = buflen - fUIO->uio_resid;
    curproc->filetable[fd]->offset += *retval;
    
    lock_release(curproc->filetable[fd]->ft_lock);
    
    return 0;
    
}


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

int sys_close(int fd){
    
    if(curproc->filetable[fd] == NULL){
        return EBADF;   
     }
     lock_acquire(curproc->filetable[fd]->ft_lock);
     
     curproc->filetable[fd]-> count--;
     VOP_DECREF(curproc->filetable[fd]->ft_vnode);
     if(curproc->filetable[fd]->count == 0){
        vfs_close(curproc->filetable[fd]->ft_vnode);
         
        lock_release(curproc->filetable[fd]->ft_lock);
        kfree(curproc->filetable[fd]);
        curproc->filetable[fd] = NULL;
        return 0;
       
     }
     
      lock_release(curproc->filetable[fd]->ft_lock);
      return 0;
}

