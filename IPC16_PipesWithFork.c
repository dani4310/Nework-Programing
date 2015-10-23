#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#define MAXBUFF 100

void err_sys(const char *errormessage);
void client(int readfd, int writefd);
void server(int readfd, int writefd);

int main(){
	int childpid, pipe1[2], pipe2[2];
	if (pipe(pipe1) < 0 || pipe(pipe2) < 0) err_sys("can't create pipes");
	if ( (childpid = fork()) < 0) {
		err_sys("can't fork");
	} else if (childpid > 0) { /* parent */
		close(pipe1[0]); close(pipe2[1]);
		client(pipe2[0], pipe1[1]);
		while (wait((int *) 0) != childpid) /* wait for child */
		;
	close(pipe1[1]); close(pipe2[0]);
	exit(0);
	} else { /* child */
		close(pipe1[1]); close(pipe2[0]);
		server(pipe1[0], pipe2[1]);
		close(pipe1[0]); close(pipe2[1]);
		exit(0);
	}

}

void client(int readfd, int writefd)
{
char buff[MAXBUFF];
int n;
// Read the filename from standard input, write it to the IPC descriptor.
if (fgets(buff, MAXBUFF, stdin) == NULL) err_sys("client: filename read error");
n = strlen(buff);
if (buff[n-1] == '\n') n--; /* ignore newline from fgets() */
if (write(writefd, buff, n) != n) err_sys("client: filename write error");
// Read the data from the IPC descriptor and write to standard output.
while ( (n = read(readfd, buff, MAXBUFF)) > 0)
if (write(1 /* stdout*/, buff, n) != n) err_sys("client: data write error");
if (n < 0) err_sys("client: data read error");
}

void server(int readfd, int writefd)
{
char buff[MAXBUFF], errmesg[256], *sys_err_str();
int n, fd;
// Read the filename from the IPC descriptor.
if ( (n = read(readfd, buff, MAXBUFF)) <= 0) err_sys("server: filename read error");
buff[n] = '\0'; /* null terminate filename */
if ( (fd = open(buff, 0)) < 0) {
// Error. Format an error message and send it back to the client.
sprintf(errmesg, ": can't open, %s\n", sys_err_str());
strcat(buff, errmesg);
n = strlen(buff);
if (write(writefd, buff, n) != n) err_sys("server: errmesg write error");
} else {
// Read the data from the file and write to the IPC descriptor.
while ( (n = read(fd, buff, MAXBUFF)) > 0)
if (write(writefd, buff, n) != n) err_sys("server: data write error");
if (n < 0) err_sys("server: read error");
}
}

void err_sys(const char *errormessage){
	printf("\033[1;31m%s\033[0m\n", errormessage);
}

char *sys_err_str()
{
extern int errno;           /* UNIX error number */
extern int sys_nerr;        /* # of error message strings in sys table */
extern const char *const sys_errlist[]; /* the system error message table */
   static char msgstr[200];
   if (errno != 0 ){
      if (errno > 0 && errno < sys_nerr)
         sprintf(msgstr,"(%s)", sys_errlist[errno]);
      else
         sprintf(msgstr,"(errno = %d)", errno);
    }
    else msgstr[0] = '\0';
    return(msgstr);
}