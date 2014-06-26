/*
 * tcpserver.c
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

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, srv_portno, cli_portno, clilen;
    char* cmd;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
	FILE *file_ptr;
  	file_ptr = fopen("config.dat", "r");
  	cmd = malloc(100 * sizeof(char));
  	bzero(cmd, 100);
	while( ( fgets(cmd, 100, file_ptr) ) != NULL ) {
		if(strncmp(cmd,"adder",5) == 0) {
			cmd = strtok(cmd, "\n");
			printf("adder cmd: %s\n",cmd);
			strtok(cmd, " ");
			strtok(NULL, " ");
			srv_portno = atoi(strtok(NULL, " "));
			printf("in if: %d\n",srv_portno);
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
//     n = write(newsockfd,"Hello this is adder and I am ready",34);
//     if (n < 0) error("ERROR writing to socket");
     while(1) {
     	  buffer = malloc(8 * sizeof(char));
   	      n = read(newsockfd,buffer,8*sizeof(char));
   	      if (n < 0) error("ERROR reading from socket");
   	      n = read(newsockfd,&field,sizeof(int));
   	      if (n < 0) error("ERROR reading from socket");
   	      printf("Adder: Instruction (%s,%d)\n",buffer,field);
   	      if(strncmp(buffer,"stopstop",8) == 0) {
   	    	  printf("Adder: Encountered end of instructions command.\n");
   	    	  break;
   	      }
   	      memset(buffer, 0, 8);
   	      free(buffer);
     }
//   	      bzero(buffer,256);
//   	      n = read(newsockfd,buffer,255);
//   	      if (n < 0) error("ERROR reading from socket");
//   	      printf("Adder: Instruction %s\n",buffer);
//   	      if(strncmp(buffer,"stopstop",8) == 0) {
//   	    	  printf("Adder: Encountered end of instructions command.\n");
//   	      }
     return 0;
}
