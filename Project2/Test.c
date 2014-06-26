/*
 * Test.c
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

volatile sig_atomic_t interrupt_rcvd_cnt = 0;

void main_sig_handler(int signum) {
	printf("Test Parent: Entered sig_handler\n");
	sleep(3);
	interrupt_rcvd_cnt++;
	printf("Test Parent: Exited sig_handler\n");
}

int main(int argc, char *argv[])
{
	int pid1;
	pid1 = fork();
	if(pid1 == 0) {
		kill(getppid(), SIGUSR1);
		kill(getppid(), SIGUSR1);
		kill(getppid(), SIGUSR1);
		kill(getppid(), SIGUSR1);
		kill(getppid(), SIGUSR1);
		sleep(10);
		printf("Child Exiting.\n");
	} else {
		signal(SIGUSR1, main_sig_handler);
		int i = 0;
		for(i = 0; i < 100000000; i++) {
			int rd = rand()*rand();
		}
		sleep(10);
		printf("Parent Exiting %d.\n",interrupt_rcvd_cnt);
	}
}
