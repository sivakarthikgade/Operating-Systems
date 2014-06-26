/*
 * BalancedSorting.c
 *
 *  Created on: Mar 15, 2014
 *      Author: sivakarthik
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>

volatile int size;
volatile int logsize;
volatile int* input;
volatile sem_t sem1;
volatile sem_t sem2;
volatile int cnt1;
volatile int cnt2;

void *compare_and_swap(void *ptr);

main(int argc, char** argv) {
	int num = 0;
	int thread_cnt = 0;
	size = 0;

	FILE *file_ptr = fopen("input.dat", "r");
	while(fscanf(file_ptr, "%d", &size) > 0) {

		if(size == 0) {
			printf("Main: Reached end of input integer lists.\n");
			break;
		}

		thread_cnt = size/2;
		logsize = log2l(size);
		sem_init((sem_t *) &sem1, 0, 0);
		sem_init((sem_t *) &sem2, 0, 0);

		input = malloc(size*sizeof(int));
		memset((int *) input, 0, size*sizeof(int));
		int i,j,z;
		for(i = 0; i < size; i++) {
			num = 0;
			fscanf(file_ptr, "%d", &num);
			input[i] = num;
		}

		printf("Initial Array State: ");
		for(z = 0; z < size; z++) {
			printf("%d,",input[z]);
		}
		printf("\n");
		cnt1 = 0;
		cnt2 = 0;
		pthread_t threads[thread_cnt];
		int args[thread_cnt];
		for(i = 0; i < thread_cnt; i++) {
			args[i] = i;
			pthread_create(&threads[i], NULL, compare_and_swap, (void *) &args[i]);
		}

		for(i = 0; i < logsize*logsize; i++) {
			while(cnt1 < thread_cnt) {}
			cnt1 = 0;
			for(j = 0; j < thread_cnt; j++) {
				sem_post((sem_t *) &sem1);
			}
			while(cnt2 < thread_cnt) {}
			cnt2 = 0;
			for(j = 0; j < thread_cnt; j++) {
				sem_post((sem_t *) &sem2);
			}
			printf("In %d stage after %d phase, Array State: ", (i/logsize)+1, (i%logsize)+1);
			for(z = 0; z < size; z++) {
				printf("%d,",input[z]);
			}
			printf("\n");
		}

		for(i = 0; i < (size/2); i++) {
			pthread_join(threads[i], NULL);
		}

		printf("Final Array State: ");
		for(z = 0; z < size; z++) {
			printf("%d,",input[z]);
		}
		printf("\n");
		memset((int *) input, 0, size*sizeof(int));
		free((int *) input);
		size = 0;

		sem_destroy((sem_t *) &sem1);
		sem_destroy((sem_t *) &sem2);
	}

	exit(0);
}

void *compare_and_swap(void *ptr) {
	int i, j, k;
	int *t;
	t = (int *) ptr;
	for(i = 1; i <= logsize; i++) {
		for(j = 1; j <= logsize; j++) {
			cnt1++;
			sem_wait((sem_t *) &sem1);
			int num_groups = pow(2,j-1);
			int group_size = size/num_groups;
			int g = (*t)/(group_size/2);
			int gindex = (*t)%(group_size/2);
			int group_start = g*group_size;
			int group_end = (g+1)*group_size - 1;
			int data1 = group_start + gindex;
			int data2 = group_end - gindex;
			if(input[data1] > input[data2]) {
				int temp = input[data1];
				input[data1] = input[data2];
				input[data2] = temp;
			}
			for(k = 1; k <= (data1+data2); k++) {}
			cnt2++;
			sem_wait((sem_t *) &sem2);
		}
	}
}
