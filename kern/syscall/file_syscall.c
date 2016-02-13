

#include <vfs.h>
#include <vnode.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/limits.h>
#include <current.h>
#include <proc.h>
#include <uio.h>
#include <kern/iovec.h>
#include <filetable.h>
#include <limits.h>
#include <file_syscall.h>



int sys_open(const char *filename, int flags, mode_t mode){
    if(flag != O_RDONLY && flag != O_WRONLY && flag != O_RDWR){
        return EINVAL; // Invalid argument flags can only be one of the three 
    }
    
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
    
    //ft_add return file descriptor (int);
    result = ft_add(fbuff,flags,mode);
    
    //if ft_add returns 1 (false)
    if(result = -1){
        kfree(kbuff);
        return result;
    } 
    return result; //returns the file descriptor
    
}

//fd: file descriptor
int sys_read(int fd , void *buf, size_t buflen){
 
    if( fd< 0 || fd >= FDS_MAX || filetable[fd] == NULL){
        return EBADF; //return bad file number
    }
    struct ftEntry file = filetable[fd];
    lock_acquire(file-> ft_lock);
    if(file -> mode = O_WRONLY){
   
      lock_release(file->ft_lock);
    }
    struct iovec *fIOV =(struct iovect*) kmalloc(sizeof(struct iovec));
    
    fIOV ->iov_ubase = (userptr_t)buf;    
    fIOV ->iov_len = buflen;
    
    struct uio *fUIO = (struct uio*)kmalloc(sizeof(struct uio));
    
    fUIO -> uio_iov = &fIOV;
    fUIO -> uio_iovcnt = 1;
    fUIO -> uio_offset = file -> offset;
    fUIO -> uio_resid = buflen;
    fUIO -> uio_segflg = UIO_USRSPACE;
    fUIO -> uio_rw = UIO_READ;
    fUIO -> addrspace = NULL; //null for now
    
    
    return 0;
   
    
    
    
  
}   
    
    
    
    
    
    
    


=======
abc
>>>>>>> 340addb3eb7c289c76ef8122b83a5814cc47803e
