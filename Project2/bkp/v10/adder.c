/*
 * adder.c
 *
 *  Created on: Feb 16, 2014
 *      Author: sivakarthik
 */


/* Adder server: Adds numbers in the file in instruction cmd mod field.
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

volatile long adder_instr_rcvd_cnt = 0;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void adder_sig_handler(int signum) {
	printf("Adder: Inside sig_handler\n");
	adder_instr_rcvd_cnt++;
}

int main(int argc, char *argv[])
{
	signal(SIGUSR1, adder_sig_handler);

	int mainpsockfd, sockfd, newsockfd, srv_portno, cli_portno, mainpportno, clilen, N, M, mainp_serv_addr_len;
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
				fprintf(stderr,"Adder: ERROR, no such host\n");
				exit(0);
			}
			mainpportno = atoi(strtok(NULL, " "));
			printf("Adder: mainpportno - %d.\n",mainpportno);
		} else if(strncmp(cmd,"adder",5) == 0) {
			cmd = strtok(cmd, "\n");
			strtok(cmd, " ");
			strtok(NULL, " ");
			srv_portno = atoi(strtok(NULL, " "));
			printf("Adder: adderportno - %d.\n",srv_portno);
		} else if(isdigit(cmd[0])) {
			cmd = strtok(cmd, "\n");
			N = atoi(strtok(cmd, " "));
			M = atoi(strtok(NULL, " "));
			printf("Adder: Value of N,M are %d,%d.\n",N,M);
		}
		bzero(cmd,100);
		free(cmd);
		cmd = malloc(100 * sizeof(char));
	}

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

     mainpsockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
     if (mainpsockfd < 0)
         error("ERROR opening socket for UDP connections.\n");
     bzero((char *) &mainp_serv_addr, sizeof(mainp_serv_addr));
     mainp_serv_addr.sin_family = AF_INET;
     bcopy((char *)mainpserver->h_addr, (char *)&mainp_serv_addr.sin_addr.s_addr, mainpserver->h_length);
     mainp_serv_addr.sin_port = htons(mainpportno);
//     inet_aton(mainpserver->h_addr, &mainp_serv_addr.sin_addr);
     mainp_serv_addr_len = sizeof(mainp_serv_addr);

     char *filename;
     int y, result;
     while(1) {
    	 while(adder_instr_rcvd_cnt < 5) continue;
		  if(((adder_instr_rcvd_cnt-instr_cmpl_cnt)>=N) && (flag==0)) {
			  kill(getppid(), SIGUSR1);
			  flag = 1;
			  printf("Adder: Sending intrpt to pause instrs.\n");
		  } else if((flag==1) && ((adder_instr_rcvd_cnt-instr_cmpl_cnt)<M)) {
			  flag = 0;
			  char* msg;
			  msg = malloc(5*sizeof(char));
			  sprintf(msg, "rsadd");
			  n = sendto(mainpsockfd, msg, 5*sizeof(char), 0, (struct sockaddr *) &mainp_serv_addr, mainp_serv_addr_len);
			  if (n < 0)
				  error("Adder: ERROR writing to socket using UDP.\n");
			  memset(msg,0,5);
			  free(msg);
			  printf("Adder: Sending ackn to resume instrs.\n");
		  }
		filename = malloc(8 * sizeof(char));
		n = read(newsockfd,filename,8*sizeof(char));
		if (n < 0) error("ERROR reading from socket");
		n = read(newsockfd,&y,sizeof(int));
		if (n < 0) error("ERROR reading from socket");
		printf("Adder: Processing (add,%s,%d).",filename,y);
		if(strncmp(filename,"stopstop",8) == 0) {
			printf(" Encountered end of instructions command.\n");
			break;
		}
		FILE *file_ptr = fopen(filename, "r");
		char *instr = malloc(100 * sizeof(char));
		fgets(instr,100,file_ptr);
		memset(instr,0,100);
		free(instr);
		instr = malloc(100*sizeof(char));
		int j;
		result = 0;
		while( ( fgets(instr,100,file_ptr) ) != NULL ) {
			instr = strtok(instr, "\n");
			j = (int) strtol(instr,(char **)NULL,10);
			result = (result+(j%y))%y;
			memset(instr,0,100);
			free(instr);
			instr = malloc(100*sizeof(char));
		}
		fclose(file_ptr);
		printf(" Sum of numbers in %s mod %d is: %d.\n",filename,y,result);
		memset(instr,0,100);
		free(instr);
		memset(filename,0,8*sizeof(char));
		free(filename);
		y = 0;
		instr_cmpl_cnt++;
	}

     char* msg;
     msg = malloc(10*sizeof(char));
     sprintf(msg, "exit_adder");
     n = sendto(mainpsockfd, msg, 10*sizeof(char), 0, (struct sockaddr *) &mainp_serv_addr, mainp_serv_addr_len);
     if (n < 0)
    	 error("Adder: ERROR writing to socket using UDP.\n");
     memset(msg, 0, 10);
     free(msg);
     printf("Adder: Sending exit instruction to main process.\n");
     printf("Adder Exiting!!! instr_rcvd_cnt: %ld, instr_cmpl_cnt: %ld.\n",adder_instr_rcvd_cnt,instr_cmpl_cnt);
     return 0;
}
