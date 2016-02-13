#include <synch.h>
#include <lib.h>
#include <current.h>
#include <vnode.h>
#include <types.h>

struct ftEntry{
    struct vnode *ft_vnode;
    off_t offset;
    mode_t mode;
    int flags;
    int count;
    struct lock *ft_lock;
};

int ft_init(void);

int ft_add(char* filename, int flags, mode_t mode);
