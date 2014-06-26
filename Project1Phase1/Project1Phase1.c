#include <stdio.h>

void main() {
	char buf[1024];
	char msg1[] = "This is Mesg11";
	char msg2[] = "This is Mesg22";
	char msg3[] = "This is Mesg33";
	int fd1[2], fd2[2], fd3[2];
	int pid1, pid2;
	pipe(fd1);
	pipe(fd2);
	pipe(fd3);
	printf("sizeof(*msg1) is: %d\n", sizeof(*msg1));

	pid1 = fork();
	if(pid1 == 0) {
		close(fd1[1]);
		close(fd2[0]);
		close(fd2[1]);
		close(fd3[0]);

		read (fd1[0], buf, sizeof (msg1));
	    printf("IM at Child1: %s\n",buf);
	    exit(0);
	} else {
		pid2 = fork();
		if(pid2 == 0) {
			close(fd1[0]);
			close(fd1[1]);
			close(fd2[1]);
			close(fd3[0]);

			read (fd2[0], buf, sizeof (msg2));
	    	printf("IM at Child2: %s\n",buf);
		    exit(0);
		} else {
			close(fd1[0]);
			close(fd2[0]);
			close(fd3[1]);

			write (fd1[1], msg1, sizeof (msg1));
			write (fd2[1], msg2, sizeof (msg2));
			read (fd3[0], buf, sizeof (msg3));
			printf("Exiting after the exit of both child processes!!!");
			exit(0);
		}
	}
}

/*void main() {
	char *file_path = "/home/sivakarthik/os/sampledoc.txt";
	FILE *input_file;
	char *ch = malloc(sizeof(*ch)*100);

	input_file = fopen(file_path,"r");
	if(input_file == 0) {
		printf("Invalid File Path!! %s", file_path);
	} else {
		printf("File Opened!! %s\n", file_path);
		while( ( fgets(ch,100,input_file) ) != NULL )
		      printf("%s",ch);
		fclose(input_file);
	}
}*/
