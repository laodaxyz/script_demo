#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct sigaction act;

/*
 * SIGINT信号处理程序
 */
 void catchctrlc(int signo){
    char msg[]    = "Ctrl-C enterred!\n";
    char errmsg[] = "Failed to restore SIGINT\n";
    int msglen    = sizeof(msg);
    int emsglen   = sizeof(errmsg);

    /* 恢复为该信号默认信号处理方式 */
    act.sa_hander = SIG_DFL;
    write(STDERR_FILENO, msg, msglen);

    /* 恢复SIGINT信号处理函数 */
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        write(STDERR_FILENO, errmsg, emsglen);
    }
 }

int main(int argc, char const *argv[])
{
    act.sa_hander = catchctrlc;  /* 设置信号处理程序 */
    act.sa_flags = 0;  /* 无特殊选项 */

    /* 清空信号掩码，设置SIGINT的信号处理函数 */
    if ((sigemptyset(&act.sa_mask) == -1) ||
        (sigaction(SIGINT, &act, NULL) == -1))
    {
        perror("Failed to set SIGINT to hander Ctrl-C");
    }

    for(;;)
        ;
    return 0;
}