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
	printf("Main: Inside sig_handler\n");
	interrupt_rcvd_cnt++;
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
			signal(SIGUSR1, main_sig_handler);
		    int mainpsockfd, addersockfd, factosockfd, newsockfd, mainpportno, adderportno, factoportno, n, D, clilen;
		    long interrupt_hndl_cnt = 0;
		    struct sockaddr_in mainp_serv_addr, adder_serv_addr, facto_serv_addr, cli_addr;
		    struct hostent *adderserver, *factoserver;

			char* buffer;
			mainpsockfd = socket(AF_INET, SOCK_STREAM, 0);
			 if (mainpsockfd < 0)
				  error("ERROR opening socket");
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
				if(strncmp(cmd,"mainp",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
					strtok(NULL, " ");
					mainpportno = atoi(strtok(NULL, " "));
					printf("Main: mainpportno - %d.\n",mainpportno);
				} else if(strncmp(cmd,"adder",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
				    adderserver = gethostbyname(strtok(NULL, " "));
				    if (adderserver == NULL) {
				        fprintf(stderr,"ERROR, no such host\n");
				        exit(0);
				    }
					adderportno = atoi(strtok(NULL, " "));
					printf("Main: adderportno - %d.\n",adderportno);
				} else if(strncmp(cmd,"facto",5) == 0) {
					cmd = strtok(cmd, "\n");
					strtok(cmd, " ");
				    factoserver = gethostbyname(strtok(NULL, " "));
				    if (factoserver == NULL) {
				        fprintf(stderr,"ERROR, no such host\n");
				        exit(0);
				    }
					factoportno = atoi(strtok(NULL, " "));
					printf("Main: factoportno - %d.\n",factoportno);
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

		     bzero((char *) &mainp_serv_addr, sizeof(mainp_serv_addr));
		     mainp_serv_addr.sin_family = AF_INET;
		     mainp_serv_addr.sin_addr.s_addr = INADDR_ANY;
		     mainp_serv_addr.sin_port = htons(mainpportno);
		     if (bind(mainpsockfd, (struct sockaddr *) &mainp_serv_addr, sizeof(mainp_serv_addr)) < 0)
		          error("ERROR on binding");
		     listen(mainpsockfd,5);

		     clilen = sizeof(cli_addr);
		     newsockfd = accept(mainpsockfd, (struct sockaddr *) &cli_addr, &clilen);
		     if (newsockfd < 0)
		          error("ERROR on accept");


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
				//Check if USR1 interrupt is set.
				while((interrupt_rcvd_cnt - interrupt_hndl_cnt) > 0) {
					interrupt_hndl_cnt++;
					char* resume_instr;
					resume_instr = malloc(12 * sizeof(char));
					n = read(newsockfd,resume_instr,12*sizeof(char));
					if (n < 0) error("ERROR reading from socket");
//					if((strncmp(resume_instr,"resume_adder",12) == 0) || (strncmp(resume_instr,"resume_facto",12) == 0)) {
//						Right now don't have a way to discriminate between who set the interrupt. Otherwise, can put in appropriate logic.
//					}
					memset(resume_instr,0,9);
					free(resume_instr);
				}
			}
			fclose(file_ptr);
			int z = 0;
			n = write(addersockfd,(char *)"stopstop",8*sizeof(char));
			if (n < 0)
				error("ERROR writing to socket");
			n = write(addersockfd,&z,sizeof(int));
			if (n < 0)
				error("ERROR writing to socket");
			n = write(factosockfd,&z,sizeof(int));
			if (n < 0)
				error("ERROR writing to socket");
			n = write(factosockfd,&z,sizeof(int));
			if (n < 0)
				error("ERROR writing to socket");

			char* dummy_instr;
			dummy_instr = malloc(10 * sizeof(char));
			n = read(newsockfd,dummy_instr,10*sizeof(char));
			if (n < 0) error("ERROR reading from socket");
			memset(dummy_instr,0,10);
			free(dummy_instr);
			printf("Main Process Exiting!!! interrupt_rcvd_cnt: %ld, interrupt_hndl_cnt: %ld.\n",interrupt_rcvd_cnt,interrupt_hndl_cnt);
		}
	}

	return 0;
}
