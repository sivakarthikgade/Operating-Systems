/*
 * main.c
 *
 *  Created on: Feb 16, 2014
 *  Author: sivakarthik
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

volatile sig_atomic_t instr_rcvd_cnt = 0;
volatile sig_atomic_t usr2_rcvd_cnt = 0;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void sig_handler1(int signum) {
	printf("Main: Inside sig_handler1\n");
	instr_rcvd_cnt++;
}

void sig_handler2(int signum) {
	printf("Main: Inside sig_handler2\n");
	usr2_rcvd_cnt++;
}

int main(int argc, char *argv[])
{
	int pid1, pid2;

	pid1 = fork();
	if(pid1 == 0) {
		printf("Main: Invoking adder process through execl call.\n");
		execl("adder","adder",NULL);
		printf("Main: Failed to invoke adder program through execl call.\n");
		_exit(1);
	} else {
		pid2 = fork();
		if(pid2 == 0) {
			printf("Main: Invoking facto process through execl call.\n");
			execl("facto","facto",NULL);
			printf("Main: Failed to invoke adder program through execl call.\n");
			_exit(1);
		} else {
			signal(SIGUSR1, sig_handler1);
			signal(SIGUSR2, sig_handler2);
		    int addersockfd, factosockfd, adderportno, factoportno, n, D;
		    struct sockaddr_in adder_serv_addr, facto_serv_addr;
		    struct hostent *adderserver, *factoserver;

			char* buffer;
		    addersockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if (addersockfd < 0)
		        error("ERROR opening socket");
		    factosockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if (factosockfd < 0)
		        error("ERROR opening socket");

		    char* cmd;
			FILE *file_ptr;
		  	file_ptr = fopen("config.dat", "r");
		  	cmd = malloc(100 * sizeof(char));
		  	bzero(cmd, 100);
			while( ( fgets(cmd, 100, file_ptr) ) != NULL ) {
				if(strncmp(cmd,"adder",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
				    adderserver = gethostbyname(strtok(NULL, " "));
				    if (adderserver == NULL) {
				        fprintf(stderr,"ERROR, no such host\n");
				        exit(0);
				    }
					adderportno = atoi(strtok(NULL, " "));
				} else if(strncmp(cmd,"facto",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
				    factoserver = gethostbyname(strtok(NULL, " "));
				    if (factoserver == NULL) {
				        fprintf(stderr,"ERROR, no such host\n");
				        exit(0);
				    }
					factoportno = atoi(strtok(NULL, " "));
				} else if(isdigit(cmd[0])) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
					strtok(NULL, " ");
					D = atoi(strtok(NULL, " "));
					printf("Main: Value of D is %d.\n",D);
				}
				bzero(cmd,100);
				free(cmd);
				cmd = malloc(100 * sizeof(char));
			}

		    bzero((char *) &adder_serv_addr, sizeof(adder_serv_addr));
		    adder_serv_addr.sin_family = AF_INET;
		    bcopy((char *)adderserver->h_addr,
		         (char *)&adder_serv_addr.sin_addr.s_addr,
		         adderserver->h_length);
		    adder_serv_addr.sin_port = htons(adderportno);
		    while (connect(addersockfd,&adder_serv_addr,sizeof(adder_serv_addr)) < 0){}
		    printf("Successfully connected to adder server.\n");

		    bzero((char *) &facto_serv_addr, sizeof(facto_serv_addr));
		    facto_serv_addr.sin_family = AF_INET;
		    bcopy((char *)factoserver->h_addr,
		         (char *)&facto_serv_addr.sin_addr.s_addr,
		         factoserver->h_length);
		    facto_serv_addr.sin_port = htons(factoportno);
		    while (connect(factosockfd,&facto_serv_addr,sizeof(facto_serv_addr)) < 0){}
		    printf("Successfully connected to facto server.\n");

		    file_ptr = fopen("instruction.dat", "r");
		    buffer = malloc(256 * sizeof(char));
			while( ( fgets(buffer,256,file_ptr) ) != NULL ) {
				buffer = strtok(buffer,"\n");
				char *command = strtok(buffer, " ");
				if(strncmp(command,"add",3) == 0) {
				    char *x = strtok(NULL, " ");
				    int y = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
					n = write(addersockfd,x,8*sizeof(char));
					if (n < 0)
						error("ERROR writing to socket");
					n = write(addersockfd,&y,sizeof(int));
					if (n < 0)
						error("ERROR writing to socket");
				    kill (pid1, SIGUSR1);
				} else if(strncmp(command,"fac",3) == 0) {
				    int x = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
				    int y = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
					n = write(factosockfd,&x,sizeof(int));
					if (n < 0)
						error("ERROR writing to socket");
					n = write(factosockfd,&y,sizeof(int));
					if (n < 0)
						error("ERROR writing to socket");
				    kill (pid2, SIGUSR1);
				} else {
					fprintf(stderr,"Invalid command in instruction.dat file: %s %s\n",command,buffer);
				}
				memset(buffer,0,256);
				free(buffer);
				buffer = malloc(256 * sizeof(char));
				int cnt = 0;
				for(cnt = 0; cnt < D; cnt++) {
					int rn = rand();
				}
			}
			fclose(file_ptr);
		}
	}

	printf("Main Process Exiting!!! usr1_rcvd_cnt: %d, usr2_rcvd_cnt: %d.\n",instr_rcvd_cnt,usr2_rcvd_cnt);
	return 0;
}
