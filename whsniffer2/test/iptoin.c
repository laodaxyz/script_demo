#include <stdio.h>
#include <string.h>

int iptoint( const char *ip )
{
    return ntohl( inet_addr( ip ) );
}

int main(int argc, char const *argv[])
{
	char *cip = "42.62.121.0";
	printf("%d\n", iptoint(cip));
	return 0;
}