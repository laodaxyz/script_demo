#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main()
{
	pid_t = child_pid;
	int i = 1;
    printf("The main program processe ID is %d\n", (int)getpid());
    child_pid = fork();
    if(child_pid != 0){
    	i = 0;
    	printf("The parent processe,with id %d and i=%d\n", (int)getppid(),i);
    	printf("The child's processe ID is %d\n", (int)child_pid);
    }else{
    	printf("The child processe,with id %d and i=%d\n", (int)getppid(),i);
    }
    return 0;
}