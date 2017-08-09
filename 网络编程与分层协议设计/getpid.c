#include <stdio.h>
#include <unistd.h>
int main()
{
    printf("The processe ID is %d\n", (int)getpid());
    printf("The parent processe ID is %d\n", (int)getppid());
    return 0;
}