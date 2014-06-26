/*
 * factorial.c
 *
 *  Created on: Feb 16, 2014
 *      Author: sivakarthik
 */


/* Factorial server: Computes factorial of passed number with mod field.
   The port number is passed as an argument */
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

volatile sig_atomic_t interrupt_rcvd_cnt = 0;
volatile sig_atomic_t instr_cmpl_cnt = 0;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void main_sig_handler(int signum) {
	printf("Facto: Inside sig_handler\n");
	interrupt_rcvd_cnt++;
}

int main(int argc, char *argv[])
{
	signal(SIGUSR1, main_sig_handler);
    int mainpsockfd, sockfd, newsockfd, mainpportno, srv_portno, cli_portno, clilen, N, M, flag;
    char* cmd;
    struct sockaddr_in mainp_serv_addr, serv_addr, cli_addr;
    struct hostent *mainpserver;
    int n;
	FILE *file_ptr;
  	file_ptr = fopen("config.dat", "r");
  	cmd = malloc(100 * sizeof(char));
  	bzero(cmd, 100);
	while( ( fgets(cmd, 100, file_ptr) ) != NULL ) {
		if(strncmp(cmd,"mainp",5) == 0) {
			cmd = strtok(cmd, "\n");
			strtok(cmd, " ");
			mainpserver = gethostbyname(strtok(NULL, " "));
			if (mainpserver == NULL) {
				fprintf(stderr,"ERROR, no such host\n");
				exit(0);
			}
			mainpportno = atoi(strtok(NULL, " "));
			printf("Facto: mainpportno - %d.\n",mainpportno);
		} else if(strncmp(cmd,"facto",5) == 0) {
			cmd = strtok(cmd, "\n");
			strtok(cmd, " ");
			strtok(NULL, " ");
			srv_portno = atoi(strtok(NULL, " "));
			printf("Facto: factoportno - %d.\n",srv_portno);
		} else if(isdigit(cmd[0])) {
			cmd = strtok(cmd, "\n");
			N = atoi(strtok(cmd, " "));
			M = atoi(strtok(NULL, " "));
			printf("Facto: Value of N,M are %d,%d.\n",N,M);
		}
		bzero(cmd,100);
		free(cmd);
		cmd = malloc(100 * sizeof(char));
	}

	 char* buffer;
	 int field;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
          error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(srv_portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
          error("ERROR on binding");
     listen(sockfd,5);

     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");

     mainpsockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (mainpsockfd < 0)
         error("ERROR opening socket");
     bzero((char *) &mainp_serv_addr, sizeof(mainp_serv_addr));
     mainp_serv_addr.sin_family = AF_INET;
     bcopy((char *)mainpserver->h_addr,
         (char *)&mainp_serv_addr.sin_addr.s_addr,
         mainpserver->h_length);
     mainp_serv_addr.sin_port = htons(mainpportno);
     while (connect(mainpsockfd,&mainp_serv_addr,sizeof(mainp_serv_addr)) < 0){}
     printf("Successfully connected to mainp server.\n");

     //     kill (getppid (), SIGUSR1);

     int x, y;
	  long result;
	  while(1) {
		  if(((interrupt_rcvd_cnt-instr_cmpl_cnt)>=N) && (flag==0)) {
			  kill(getppid(), SIGUSR1);
			  flag = 1;
		  } else if((flag==1) && ((interrupt_rcvd_cnt-instr_cmpl_cnt)<M)) {
			  flag = 0;
//			  kill(getppid(), SIGUSR2);
			  n = write(mainpsockfd,(char *)"resume_facto",12*sizeof(char));
			  if (n < 0)
				  error("ERROR writing to socket");
		  }
		  n = read(newsockfd,&x,sizeof(int));
		  if (n < 0) error("ERROR reading from socket");
		  n = read(newsockfd,&y,sizeof(int));
		  if (n < 0) error("ERROR reading from socket");
		  printf("Factorial: Processing (fac,%d,%d).",x,y);
		  if(y ==0) {
			printf(" Encountered end of instructions command.\n");
			break;
		  }
		  if(x == 0 || x >= y) {
			  result = 0;
		  } else {
			  int j;
			  result = 1;
			  for(j = 1; j <= x; j++) {
				  result = (result*j)%y;
			  }
		  }
		  printf(" Factorial of %d mod %d is: %ld.\n",x,y,result);
		  instr_cmpl_cnt++;
	  }

	printf("Factorial Exiting!!! instr_rcvd_cnt: %d, instr_cmpl_cnt: %d.\n",interrupt_rcvd_cnt,instr_cmpl_cnt);
    return 0;
}
