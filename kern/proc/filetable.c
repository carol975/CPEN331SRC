#include <filetable.h>
#include <types.h>
#include <lib.h>
#include <kern/errno.h>
#include <vfs.h>
#include <vnode.h>
#include <current.h>
#include <proc.h>
#include <kern/fcntl.h>

/*
 * filetable[0]: standard input (stdin)
 * filetable[1]: standard output (stdout)
 * filetable[2]: standard err (stderr)
 *
 *
 */

int ft_init(void){
    
    int result;
    struct vnode * vn;
    
    char *in = kstrdup("con:");
    char *out = kstrdup("con:");
    char *err = kstrdup("con:");
    
    //standard input
    curproc->filetable[0] = (struct ftEntry* )kmalloc(sizeof(struct ftEntry));
    result = vfs_open(in, O_RDWR, 0, &vn); 
    if(result){
        return result;
    }
    curproc->filetable[0]->ft_vnode = vn;
    curproc->filetable[0]->offset = 0;
    curproc->filetable[0]->mode = 0;
    curproc->filetable[0]->flags = O_RDWR;
    curproc->filetable[0]->count = 1;
    curproc->filetable[0]->ft_lock = lock_create(in);
    
    
    //stdard output
    curproc->filetable[1] = (struct ftEntry* )kmalloc(sizeof(struct ftEntry));
    result = vfs_open(out, O_RDWR, 0, &vn); 
    if(result){
        return result;
    }
    curproc->filetable[1]->ft_vnode = vn;
    curproc->filetable[1]->offset = 0;
    curproc->filetable[1]->mode = 0;
    curproc->filetable[1]->flags = O_RDWR;
    curproc->filetable[1]->count = 1;
    curproc->filetable[1]->ft_lock = lock_create(out);
    
    
    //stardard error
    curproc->filetable[2] = (struct ftEntry* )kmalloc(sizeof(struct ftEntry));
    result = vfs_open(err, O_RDWR, 0, &vn); 
    if(result){
        return result;
    }
    curproc->filetable[2]->ft_vnode = vn;
    curproc->filetable[2]->offset = 0;
    curproc->filetable[2]->mode = 0;
    curproc->filetable[2]->flags = O_RDWR;
    curproc->filetable[2]->count = 1;
    curproc->filetable[2]->ft_lock = lock_create(err);
    
    int i;
    for(i = 3; i < FDS_MAX; i++){
        curproc->filetable[i] = NULL;
    }
    
    return 0;
}

int ft_add(char* filename, int flags, mode_t mode){

    struct vnode *vn;
    int index = 3;
    int result;
    
    
    //check for empty space
    while(curproc->filetable[index] != NULL){
        index++;
        if(index == FDS_MAX){
            return ENFILE;
        }
    }
    
    
    result = vfs_open(filename, flags, mode, &vn);
    if(result){
        kfree(filename);
        kfree(curproc->filetable[index]);
        return result;
    }
    
    
    curproc->filetable[index] = (struct ftEntry* )kmalloc(sizeof(struct ftEntry));
    curproc->filetable[index]->ft_vnode = vn;
    curproc->filetable[index]->offset = 0;
    curproc->filetable[index]->flags = flags;
    curproc->filetable[index]->count = 1;
    curproc->filetable[index]->ft_lock = lock_create(filename);

    return index;
}
