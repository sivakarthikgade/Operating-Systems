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

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	char* buffer;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    char* cmd;
	FILE *file_ptr;
  	file_ptr = fopen("config.dat", "r");
  	cmd = malloc(100 * sizeof(char));
  	bzero(cmd, 100);
	while( ( fgets(cmd, 100, file_ptr) ) != NULL ) {
		if(strncmp(cmd,"adder",5) == 0) {
			cmd = strtok(cmd, "\n");
			printf("in if: %s\n",cmd);
			strtok(cmd, " ");
		    server = gethostbyname(strtok(NULL, " "));
		    if (server == NULL) {
		        fprintf(stderr,"ERROR, no such host\n");
		        exit(0);
		    }
			portno = atoi(strtok(NULL, " "));
		}
		bzero(cmd,100);
		free(cmd);
		cmd = malloc(100 * sizeof(char));
	}

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    while (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0){}
    printf("successfully connected\n");

//		READ CODE SNIPPET
//    buffer = malloc(256 * sizeof(char));
//    n = read(sockfd,buffer,255);
//    if (n < 0)
//         error("ERROR reading from socket");
//    printf("%s\n",buffer);
//    memset(buffer,0,256);
//    free(buffer);


	file_ptr = fopen("instruction.dat", "r");
    buffer = malloc(256 * sizeof(char));
	while( ( fgets(buffer,256,file_ptr) ) != NULL ) {
		buffer = strtok(buffer,"\n");
		if(strncmp(strtok(buffer, " "),"add",3) == 0) {
			n = write(sockfd,strtok(NULL, " "),8*sizeof(char));
			if (n < 0)
				error("ERROR writing to socket");
			int ii = atoi(strtok(NULL, " "));
			printf("ii: %d\n",ii);
			n = write(sockfd,&ii,sizeof(int));
			if (n < 0)
				error("ERROR writing to socket");
		}
		memset(buffer,0,256);
		free(buffer);
		buffer = malloc(256 * sizeof(char));
	}
	fclose(file_ptr);
//	WRITE CODE SNIPPET
//    buffer = malloc(256 * sizeof(char));
//    buffer = "One";
//    printf("One %d\n",strlen(buffer));
//	  n = write(sockfd,"One",3);
//	  if (n < 0)
//		 error("ERROR writing to socket");
//    memset(buffer,0,256);
//	  free(buffer);

	return 0;
}
