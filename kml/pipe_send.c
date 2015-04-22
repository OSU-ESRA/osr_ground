#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

/*    */
int main()
{
	int pipeout = open("/tmp/rocket_pipe", O_WRONLY);

	int i;
	for (i = 0; i < 10; i++) {
		char* line;
		sprintf(line, "%d\n", i);
		printf("Send: %s", line);
		write(pipeout, line, strlen(line));
		sleep(1);
	}

	close(pipeout);
	return 0;
}
