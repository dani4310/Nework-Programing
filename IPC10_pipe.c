#include <stdio.h>
#include <stdlib.h>

int main()
{
	int pipefd[2], n;
	char buff[100];

	if (pipe(pipefd) <0)
		printf("pipe error");

	printf ("read fd = %d, write fd %d\n", pipefd[0], pipefd[1]);
	if (write(pipefd[1], "hello world\n", 12 ) != 12)
		printf("write error");

	if ( (n = read(pipefd[0], buff, sizeof(buff))) <= 0)
		printf("read error");

	write(1, buff, n);
	exit(0);
}