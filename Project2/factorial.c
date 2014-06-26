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

volatile long facto_instr_rcvd_cnt = 0;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void facto_sig_handler(int signum) {
	printf("Factor: Received interrupt - A new instruction is submitted for processing.\n");
	facto_instr_rcvd_cnt++;
}

void main(int argc, char *argv[])
{
	signal(SIGUSR1, facto_sig_handler);

	int mainpsockfd, sockfd, newsockfd, srv_portno, cli_portno, mainpportno, clilen, N, M;
    int flag = 0;
	long instr_cmpl_cnt = 0;
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
				fprintf(stderr,"Factor: ERROR, no such main host\n");
				exit(0);
			}
			mainpportno = atoi(strtok(NULL, " "));
//			printf("Facto: mainpportno - %d.\n",mainpportno);
		} else if(strncmp(cmd,"facto",5) == 0) {
			cmd = strtok(cmd, "\n");
			strtok(cmd, " ");
			strtok(NULL, " ");
			srv_portno = atoi(strtok(NULL, " "));
//			printf("Facto: factoportno - %d.\n",srv_portno);
		} else if(isdigit(cmd[0])) {
			cmd = strtok(cmd, "\n");
			N = atoi(strtok(cmd, " "));
			M = atoi(strtok(NULL, " "));
//			printf("Facto: Value of N,M are %d,%d.\n",N,M);
		}
		bzero(cmd,100);
		free(cmd);
		cmd = malloc(100 * sizeof(char));
	}

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
          error("Factor: ERROR creating socket for TCP.\n");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(srv_portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
          error("Factor: ERROR on binding socket for TCP.\n");
     listen(sockfd,5);

     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0)
          error("Factor: ERROR on accept TCP socket.\n");

     mainpsockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
     if (mainpsockfd < 0)
         error("Factor: ERROR creating socket for UDP connections.\n");
     bzero((char *) &mainp_serv_addr, sizeof(mainp_serv_addr));
     mainp_serv_addr.sin_family = AF_INET;
     bcopy((char *)mainpserver->h_addr, (char *)&mainp_serv_addr.sin_addr.s_addr, mainpserver->h_length);
     mainp_serv_addr.sin_port = htons(mainpportno);
//     inet_aton(mainpserver->h_addr, &mainp_serv_addr.sin_addr);

     int x, y;
     long result;
     while(1) {
//    	 while(facto_instr_rcvd_cnt < 5) continue;
    	 if(((facto_instr_rcvd_cnt-instr_cmpl_cnt)>=N) && (flag==0)) {
			  kill(getppid(), SIGUSR1);
			  flag = 1;
			  printf("Factor: Sending interrupt to pause incoming instructions.\n");
    	 } else if((flag==1) && ((facto_instr_rcvd_cnt-instr_cmpl_cnt)<M)) {
			  flag = 0;
			  char* msg;
			  msg = malloc(5*sizeof(char));
			  sprintf(msg, "rsfac");
			  n = sendto(mainpsockfd, msg, 5*sizeof(char), 0, (struct sockaddr *) &mainp_serv_addr, sizeof(mainp_serv_addr));
			  if (n < 0)
				  error("Factor: ERROR writing resume acknowledgment to socket using UDP connection.\n");
			  memset(msg, 0, 5);
			  free(msg);
			  printf("Factor: Sending acknowledgment to resume incoming instructions.\n");
    	 }
    	 n = read(newsockfd,&x,sizeof(int));
    	 if (n < 0) error("Factor: ERROR reading instruction from socket.\n");
    	 n = read(newsockfd,&y,sizeof(int));
    	 if (n < 0) error("Factor: ERROR reading instruction from socket.\n");
    	 printf("Factor: Processing (fac,%d,%d).\n",x,y);
    	 if(y ==0) {
    		 printf("Factor: Encountered end of instructions command.\n");
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
    	 printf("Factor: Factorial of %d mod %d is: %ld.\n",x,y,result);
    	 x = 0;
    	 y = 0;
    	 instr_cmpl_cnt++;
     }

     char* msg;
     msg = malloc(10*sizeof(char));
     sprintf(msg, "exit_facto");
     n = sendto(mainpsockfd, msg, 10*sizeof(char), 0, (struct sockaddr *) &mainp_serv_addr, sizeof(mainp_serv_addr));
     if (n < 0)
    	 error("Factor: ERROR writing exit acknowledgment to socket using UDP.\n");
     memset(msg, 0, 10);
     free(msg);
     printf("Factor Process Exiting!!!\n");
     exit(0);
}
