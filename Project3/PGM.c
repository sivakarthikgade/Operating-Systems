/*
 * PGM.c
 *
 *  Created on: Apr 8, 2014
 *      Author: sivakarthik
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>

int* sem_init_vals = malloc(4*sizeof(int));
main(int argc, char** argv) {
	FILE *seminit_file_ptr = fopen("semainit.dat", "r");
	fscanf(seminit_file_ptr, "%d", (int *) &sem_init_vals[0]);
}
