/*
 * Driver code for airballoon problem
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>

#include <synch.h>

#define NROPES 16
static int ropes_left = NROPES;

static int index = 0;

#define true 1
#define false 0

// Data structures for rope mappings

struct Hook * Hooks;
struct Stake * Stakes;
struct Rope * Ropes;


struct Rope{
    struct lock * rp_lock;
    struct Stake * stake;
    struct Hook  * hook;
    int rope_num;
    int severed;
};

struct Stake{
    int stake_id;
};

struct Hook{
    int hook_id;
};

// Synchronization primitives 

static struct semaphore *waitsem;
static struct semaphore *finalsem;

//this is the lock for ropes_left
struct lock * total_rp_lk;

/*
 * Describe your design and any invariants or locking protocols 
 * that must be maintained. Explain the exit conditions. How
 * do all threads know when they are done?  
 */


static
void
dandelion(void *p, unsigned long arg){

	(void)p;
	(void)arg;
	
	kprintf("Dandelion thread starting\n");
	
    int randi;
	int hookID;
	
	while(ropes_left !=0){
	randi = random()%NROPES;
	lock_acquire(Ropes[randi].rp_lock);
	
	if(Ropes[randi].severed != true){
	    hookID = Ropes[randi].hook->hook_id;
	    Ropes[randi].hook = NULL;
	    Ropes[randi].severed = true;
	    
	    lock_acquire(total_rp_lk);
	    -- ropes_left;
	    lock_release(total_rp_lk);
	    
	    kprintf("Dandelion severed rope %d from hook %d\n",randi,hookID); 
	}
	
	lock_release(Ropes[randi].rp_lock);
	thread_yield();
	}
	
    kprintf("Dandelion thread done\n");
	V(waitsem);
	
    
}

static
void
marigold(void *p, unsigned long arg)
{
    (void)p;
	(void)arg;
	
	kprintf("Marigold thread starting\n");
	
	int randi;
	int stakeID;
	
	while(ropes_left !=0){
	randi = random()%NROPES;
	lock_acquire(Ropes[randi].rp_lock);
	
	if(Ropes[randi].severed != true){
	    stakeID = Ropes[randi].stake->stake_id;
	    Ropes[randi].stake = NULL;
	    Ropes[randi].severed = true;
	    lock_acquire(total_rp_lk);
	    -- ropes_left;
	    lock_release(total_rp_lk);
	    kprintf("Marigold severed rope %d from stake %d\n",randi,stakeID); 
	}
	
	lock_release(Ropes[randi].rp_lock);
	thread_yield();
	}
    
    kprintf("Marigold thread done\n");
	V(waitsem);
}

static
void
flowerkiller(void *p, unsigned long arg)
{
    (void)p;
	(void)arg;
	
	kprintf("Lord FlowerKiller thread starting\n");
	
	int randi;
	int rands;
	int stakeID;
	int old_stakeID;

	
	while(ropes_left !=0){
	randi = random()%NROPES;
	rands = random()%NROPES;
	
	lock_acquire(Ropes[randi].rp_lock);
	
	if(Ropes[randi].severed != true){
	    old_stakeID = Ropes[randi].stake->stake_id;
	    
	    //tie the rope to another random stake
	    Ropes[randi].stake = Stakes+rands;
        stakeID = Ropes[randi].stake->stake_id;
	    
	    kprintf("Lord FLowerKiller switched rope %d from stake %d to stake %d\n",randi,old_stakeID,stakeID); 
	}
	
	lock_release(Ropes[randi].rp_lock);
	thread_yield();
    
    }
    
    kprintf("Lord FLowerKiller thread done\n");
	V(waitsem);
}

static
void
balloon(void *p, unsigned long arg)
{
	(void)p;
	(void)arg;
	
	kprintf("Balloon thread starting\n");
	
	
	while(ropes_left!=0 ){
        
	}
	kprintf("Ballon freed and Prince Dandelion escapes!\n");
    kprintf("Ballon thread done\n");
	V(finalsem);
	
	  

}


// Change this function as necessary
int
airballoon(int nargs, char **args)
{

	int err = 0;
    int i;
	(void)nargs;
	(void)args;

	
	char total_rp_lk_name = 'l';
	ropes_left = NROPES;
	total_rp_lk = lock_create(&total_rp_lk_name);
	waitsem = sem_create("waitsem",0);
	finalsem = sem_create("finalsem",0);
	
	/*
	 *The functions below allocates and initialize an array of ropes_left number of Rope
    */

	// Create 16 Stakes, Hooks, and Ropes
	Stakes = kmalloc(sizeof(struct Stake)*NROPES);
	Hooks = kmalloc(sizeof(struct Hook)*NROPES);
	Ropes = kmalloc(sizeof(struct Rope) *NROPES);
	for(i = 0; i<NROPES;++i){
	    
	    // initialize the hook structs
	    Hooks[i].hook_id = i;
	    
	    // initialize the stake structs
	    Stakes[i].stake_id = i;
	    
	    // initialize the rope structs;
	    Ropes[i].rope_num = i;
        
        char temp = (char)i;
        
        Ropes[i].rp_lock = lock_create(&temp);

        Ropes[i].stake = Stakes+i;
        
        Ropes[i].hook =  Hooks+i;
        
        Ropes[i].severed = false;
	}


    
    err = thread_fork("Marigold Thread",
			  NULL, marigold, NULL, 0);
	if(err)
		goto panic;
	
	
	err = thread_fork("Dandelion Thread",
			  NULL, dandelion, NULL, 0);
	if(err)
		goto panic;
	
	err = thread_fork("Lord FlowerKiller Thread",
			  NULL, flowerkiller, NULL, 0);
	if(err)
		goto panic;

	err = thread_fork("Air Balloon",
			  NULL, balloon, NULL, 0);
	if(err)
		goto panic;

	goto done;
panic:
	panic("airballoon: thread_fork failed: %s)\n",
	      strerror(err));
	
done:
    P(waitsem);
    P(waitsem);
    P(waitsem);
    P(finalsem);
   
    sem_destroy(waitsem);
    waitsem = NULL;
    
    sem_destroy(finalsem);
    finalsem = NULL;
    
    //free rope locks
    for (index=0; index<NROPES; ++index){
        lock_destroy (Ropes[index].rp_lock);
        
    }   
    
    lock_destroy(total_rp_lk); 
    kfree(Stakes);
    kfree(Hooks);
    kfree(Ropes);
    

    kprintf("Main thread done");
	return 0;
}
