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
volatile sem_t bnd_lock;
volatile int* wt_thr_cnt;

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
//		sem_init((sem_t *) &bnd_lock, 0, 0);

		input = malloc(size*sizeof(int));
		memset((int *) input, 0, size*sizeof(int));
		int i;
		for(i = 0; i < size; i++) {
			num = 0;
			fscanf(file_ptr, "%d", &num);
			input[i] = num;
		}

		wt_thr_cnt = malloc(thread_cnt*sizeof(int));
		memset((int *)wt_thr_cnt, 0, thread_cnt*sizeof(int));

		pthread_t threads[thread_cnt];
		int args[thread_cnt];
		for(i = 0; i < thread_cnt; i++) {
			args[i] = i;
			pthread_create(&threads[i], NULL, compare_and_swap, (void *) &args[i]);
		}

		for(i = 0; i < logsize*logsize; i++) {
			char isBnRchd = 0;
			while(isBnRchd != 1) {
				isBnRchd = 1;
				int k;
				for(k = 0; k < thread_cnt; k++) {
					if(wt_thr_cnt[k] == 0) {
						isBnRchd = 0;
						break;
					}
				}
			}
			memset((int *)wt_thr_cnt, 0, thread_cnt*sizeof(int));
//			int j;
//			for(j = 0; j < thread_cnt; j++) {
//				sem_post((sem_t *) &bnd_lock);
//			}
		}

		for(i = 0; i < (size/2); i++) {
			pthread_join(threads[i], NULL);
		}

		printf("Sorted Array: ");
		for(i = 0; i < size; i++) {
			printf("%d,",input[i]);
		}
		printf("\n");
		memset((int *) wt_thr_cnt, 0, thread_cnt*sizeof(int));
		free((int *) wt_thr_cnt);
		memset((int *) input, 0, size*sizeof(int));
		free((int *) input);
		size = 0;

//		sem_destroy((sem_t *) &bnd_lock);
	}

	exit(0);
}

void *compare_and_swap(void *ptr) {
	int i, j, k;
	int *t;
	t = (int *) ptr;
	for(i = 1; i <= logsize; i++) {
		for(j = 1; j <= logsize; j++) {
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
			wt_thr_cnt[(*t)] = 1;
			while(wt_thr_cnt[(*t)] != 0){}
//			sem_wait((sem_t *) &bnd_lock);
		}
	}
}
