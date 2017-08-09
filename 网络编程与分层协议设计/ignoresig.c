#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	struct sigaction act;
	memset($act, 0, sizeof(act));
	act.sa_flags = 0;
	act.sa_hander = SIG_IGN;

	printf("SIGTSTP is ignored.\n");
	/* 忽略SIGTSTP信号，即Ctrl+z */
	if (sigaction(SIGTSTP, &act, NULL) == -1)
	{
		/* code */
	}
	return 0;
}