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
#include <ctype.h>

volatile long interrupt_rcvd_cnt = 0;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void main_sig_handler(int signum) {
	printf("Main: Received interrupt to pause submitting new instructions.\n");
	interrupt_rcvd_cnt++;
}

void main(int argc, char *argv[])
{
	int pid1, pid2;

	pid1 = fork();
	if(pid1 == 0) {
		printf("Main: Invoking adder.exe process through exec call.\n");
		execl("adder.exe","adder.exe",NULL);
		printf("Main: Failed to invoke adder.exe program through exec call.\n");
		_exit(1);
	} else {
		pid2 = fork();
		if(pid2 == 0) {
			printf("Main: Invoking factor.exe process through exec call.\n");
			execl("factor.exe","factor.exe",NULL);
			printf("Main: Failed to invoke factor.exe program through exec call.\n");
			_exit(1);
		} else {
			signal(SIGUSR1, main_sig_handler);
		    int mainpsockfd, addersockfd, factosockfd, newsockfd, mainpportno, adderportno, factoportno, n, D;
		    long interrupt_hndl_cnt = 0;
		    struct sockaddr_in mainp_serv_addr, adder_serv_addr, facto_serv_addr, cli_addr;
		    struct hostent *adderserver, *factoserver;
		    int clilen = sizeof(cli_addr);

			char* buffer;
			mainpsockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			 if (mainpsockfd < 0)
				  error("Main: ERROR creating UDP socket");
		    addersockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if (addersockfd < 0)
		        error("Main: ERROR creating adder TCP socket");
		    factosockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if (factosockfd < 0)
		        error("Main: ERROR creating factor TCP socket");

		    char* cmd;
			FILE *file_ptr;
		  	file_ptr = fopen("config.dat", "r");
		  	cmd = malloc(100 * sizeof(char));
		  	bzero(cmd, 100);
			while( ( fgets(cmd, 100, file_ptr) ) != NULL ) {
				if(strncmp(cmd,"mainp",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
					strtok(NULL, " ");
					mainpportno = atoi(strtok(NULL, " "));
//					printf("Main: mainpportno - %d.\n",mainpportno);
				} else if(strncmp(cmd,"adder",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
				    adderserver = gethostbyname(strtok(NULL, " "));
				    if (adderserver == NULL) {
				        fprintf(stderr,"Main: ERROR, no such adder host\n");
				        exit(0);
				    }
					adderportno = atoi(strtok(NULL, " "));
//					printf("Main: adderportno - %d.\n",adderportno);
				} else if(strncmp(cmd,"facto",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
				    factoserver = gethostbyname(strtok(NULL, " "));
				    if (factoserver == NULL) {
				        fprintf(stderr,"Main: ERROR, no such factor host\n");
				        exit(0);
				    }
					factoportno = atoi(strtok(NULL, " "));
//					printf("Main: factoportno - %d.\n",factoportno);
				} else if(isdigit(cmd[0])) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
					strtok(NULL, " ");
					D = atoi(strtok(NULL, " "));
//					printf("Main: Value of D is %d.\n",D);
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
		     while (connect(addersockfd,(struct sockaddr *) &adder_serv_addr,sizeof(adder_serv_addr)) < 0){}
		     printf("Main: Successfully connected to adder server.\n");

		    bzero((char *) &facto_serv_addr, sizeof(facto_serv_addr));
		    facto_serv_addr.sin_family = AF_INET;
		    bcopy((char *)factoserver->h_addr,
		         (char *)&facto_serv_addr.sin_addr.s_addr,
		         factoserver->h_length);
		    facto_serv_addr.sin_port = htons(factoportno);
		    while (connect(factosockfd,(struct sockaddr *) &facto_serv_addr,sizeof(facto_serv_addr)) < 0){}
		    printf("Main: Successfully connected to factor server.\n");

		     bzero((char *) &mainp_serv_addr, sizeof(mainp_serv_addr));
		     mainp_serv_addr.sin_family = AF_INET;
		     mainp_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		     mainp_serv_addr.sin_port = htons(mainpportno);
		     if (bind(mainpsockfd, (struct sockaddr *) &mainp_serv_addr, sizeof(mainp_serv_addr)) < 0)
		          error("Main: ERROR on binding socket for UDP connections.\n");


		    file_ptr = fopen("instruction.dat", "r");
		    buffer = malloc(256 * sizeof(char));
			while( ( fgets(buffer,256,file_ptr) ) != NULL ) {
				buffer = strtok(buffer,"\n");
				char *command = strtok(buffer, " ");
				if(strncmp(command,"add",3) == 0) {
				    char *x = strtok(NULL, " ");
				    int y = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
				    printf("Main: Submitting add(%s,%d) instruction to Adder server.\n",x,y);
					n = write(addersockfd,x,8*sizeof(char));
					if (n < 0)
						error("Main: ERROR writing instruction to socket.\n");
					n = write(addersockfd,&y,sizeof(int));
					if (n < 0)
						error("Main: ERROR writing instruction to socket.\n");
				    kill (pid1, SIGUSR1);
				} else if(strncmp(command,"fac",3) == 0) {
				    int x = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
				    int y = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
				    printf("Main: Submitting fac(%d,%d) instruction to Factor server.\n",x,y);
					n = write(factosockfd,&x,sizeof(int));
					if (n < 0)
						error("Main: ERROR writing instruction to socket.\n");
					n = write(factosockfd,&y,sizeof(int));
					if (n < 0)
						error("Main: ERROR writing instruction to socket.\n");
				    kill (pid2, SIGUSR1);
				} else {
					fprintf(stderr,"Main: Invalid command in instruction.dat file: %s %s\n",command,buffer);
				}
				memset(buffer,0,256);
				free(buffer);
				buffer = malloc(256 * sizeof(char));
				int cnt = 0;
				for(cnt = 0; cnt < D; cnt++) {
					int rn = rand();
				}
				//Check if USR1 interrupt is set.
				while((interrupt_rcvd_cnt - interrupt_hndl_cnt) > 0) {
					interrupt_hndl_cnt++;
					char* resume_instr;
					resume_instr = malloc(5 * sizeof(char));
				    n = recvfrom(mainpsockfd, resume_instr, 5, 0, (struct sockaddr *) &cli_addr, &clilen);
				    if (n < 0)
				    	error("Main: ERROR on receiving data on socket using UDP.\n");
					if((strncmp(resume_instr,"rsadd",5) == 0) || (strncmp(resume_instr,"rsfac",5) == 0)) {
						printf("Main: Received acknowledgement to resume instruction flow %s.\n",resume_instr);
					}
					memset(resume_instr,0,5);
					free(resume_instr);
				}
			}
			fclose(file_ptr);
			int z = 0;
		    printf("Main: Submitting termination instruction to Adder server.\n");
			n = write(addersockfd,(char *)"stopstop",8*sizeof(char));
			if (n < 0)
				error("Main: ERROR writing exit instruction to adder socket");
			n = write(addersockfd,&z,sizeof(int));
			if (n < 0)
				error("Main: ERROR writing exit instruction to adder socket");
		    printf("Main: Submitting termination instruction to Factor server.\n");
			n = write(factosockfd,&z,sizeof(int));
			if (n < 0)
				error("Main: ERROR writing exit instruction to factor socket");
			n = write(factosockfd,&z,sizeof(int));
			if (n < 0)
				error("Main: ERROR writing exit instruction to factor socket");

			char* exit_instr;
			exit_instr = malloc(10 * sizeof(char));
		    n = recvfrom(mainpsockfd, exit_instr, 10, 0, (struct sockaddr *) &cli_addr, &clilen);
		    if (n < 0)
		    	error("Main: ERROR on receiving exit acknowledgment.\n");
			memset(exit_instr,0,10);
			free(exit_instr);
			exit_instr = malloc(10 * sizeof(char));
		    n = recvfrom(mainpsockfd, exit_instr, 10, 0, (struct sockaddr *) &cli_addr, &clilen);
		    if (n < 0)
		    	error("Main: ERROR on receiving exit acknowledgment.\n");
			memset(exit_instr,0,10);
			free(exit_instr);
			printf("Main Process Exiting!!!\n");
		}
	}

	exit(0);
}
