/*
 * Test2.c
 *
 *  Created on: Jan 20, 2014
 *      Author: sivakarthik
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFLEN 512
#define NPACK 10
#define PORT 9930
#define SRV_IP "127.0.0.1"

void diep(char *s)
{
  perror(s);
  exit(1);
}

int main(int argc, char *argv[])
{
	int pid1, pid2;
	pid1 = fork();
	if(pid1 == 0) {
		struct sockaddr_in si_other;
		int s, i, slen=sizeof(si_other);
		char buf[BUFLEN];

		if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		  diep("socket");

		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(PORT);
		if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
		  fprintf(stderr, "inet_aton() failed\n");
		  exit(1);
		}

		for (i=0; i<NPACK; i++) {
			//sleep(1);
		  printf("Child1: Sending packet %d\n", i);
		  sprintf(buf, "This is packet %d\n", i);
		  if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, slen)==-1)
			diep("sendto()");
		}

		close(s);
 		printf("Child Exiting.\n");
		_exit(0);
	} else {
		pid2 = fork();
		if(pid2 == 0) {
			struct sockaddr_in si_other;
			int s, i, slen=sizeof(si_other);
			char buf[BUFLEN];

			if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
			  diep("socket");

			memset((char *) &si_other, 0, sizeof(si_other));
			si_other.sin_family = AF_INET;
			si_other.sin_port = htons(PORT);
			if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
			  fprintf(stderr, "inet_aton() failed\n");
			  exit(1);
			}

			for (i=0; i<NPACK; i++) {
				//sleep(1);
			  printf("Child2: Sending packet %d\n", (i+10));
			  sprintf(buf, "This is packet %d\n", (i+10));
			  if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, slen)==-1)
				diep("sendto()");
			}

			close(s);
	 		printf("Child Exiting.\n");
			_exit(0);
		} else {
			struct sockaddr_in si_me, si_other;
			int s, i, slen=sizeof(si_other);
			char buf[BUFLEN];

			if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
			  diep("socket");

			memset((char *) &si_me, 0, sizeof(si_me));
			si_me.sin_family = AF_INET;
			si_me.sin_port = htons(PORT);
			si_me.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me))==-1)
				diep("bind");

			for (i=0; i<NPACK*2; i++) {
			  if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)==-1)
				diep("recvfrom()");
			  printf("Received packet from %s:%d\nData: %s\n\n",
					 inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
			}

			close(s);
			return 0;
			printf("Parent Exiting.\n");
		}
	}
}
