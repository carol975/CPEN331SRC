#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

int
main()
{
 printf("hello world (pid:%d)\n", (int) getpid());

 int rc = fork();
 // both
 // printf("fork returns:%d\n", rc);
 if (rc < 0) { // fork failed; exit
 errx(1, "fork failed\n");
 // exit(1);
 } else if (rc == 0) { // child (new process)
 printf("hello, I am child (pid:%d)\n", (int) getpid());
 //execv
 // char *myargs[3];
 // myargs[0] = (char *)"/bin/cat"; 
 // myargs[1] = (char *)"f1"; 
 // myargs[2] = NULL; // marks end of array
 // execv(myargs[0], myargs);
 } else { // parent goes down this path (main)
 //  int status = 0;
 // waitpid(rc, &status , 0);
 printf("hello, I am parent of %d (pid:%d)\n",
 rc, (int) getpid());
 }
 printf("hello, I am done (pid:%d)\n", (int) getpid());
 return 0;
}
