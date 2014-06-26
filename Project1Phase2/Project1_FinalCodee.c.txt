/*
 * Project1Phase2.c
 *
 *  Created on: Jan 20, 2014
 *      Author: Sivakarthik Gade
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char *argv[]) {

  int pipe_proc1_instr[2], pipe_proc1_ackn[2], pipe_proc2_instr[2], pipe_proc2_ackn[2], pipe_parent_wait[2];
  int pid1, pid2;
  char *instr;
  FILE *file_ptr;
  int i, N1, N2;
  N1 = 0;
  N2 = 0;


  if (argc < 2) {
	fprintf(stderr, "Please pass value of N through command line argument.\n");
	return;
  }

  const int N = strtol(argv[1], (char **)NULL, 10);
  if (N <= 0) {
	fprintf(stderr, "N can only be positive. Please pass valid value of N through command line argument.\n");
	return;
  }

  printf("Value of N: %d\n", N);

  pipe(pipe_proc1_instr);
  pipe(pipe_proc1_ackn);
  pipe(pipe_proc2_instr);
  pipe(pipe_proc2_ackn);
  pipe(pipe_parent_wait);

  pid1 = fork();
  if(pid1 == 0) {
	  close(pipe_proc1_instr[1]);
	  close(pipe_proc1_ackn[0]);
	  close(pipe_proc2_instr[0]);
	  close(pipe_proc2_instr[1]);
	  close(pipe_proc2_ackn[0]);
	  close(pipe_proc2_ackn[1]);
	  close(pipe_parent_wait[0]);

	  int x, y;
	  long result;
	  while(1) {
		  read(pipe_proc1_instr[0], &x, sizeof(int));
		  read(pipe_proc1_instr[0], &y, sizeof(int));
		  printf("SubProcess1: Processing (fac,%d,%d).",x,y);
		  if(y ==0) {
			printf(" Encountered end of instructions command.\n");
			write(pipe_proc1_ackn[1], "d", 1);
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
		  printf(" Factorial of %d mod %d is: %d.\n",x,y,result);
		  write(pipe_proc1_ackn[1], "d", 1);
	  }

	  printf("%s\n", "SubProcess1 Exiting!!!");
	  exit(0);
  } else {
	  pid2 = fork();
	  if(pid2 == 0) {
		close(pipe_proc1_instr[0]);
		close(pipe_proc1_instr[1]);
		close(pipe_proc1_ackn[0]);
		close(pipe_proc1_ackn[1]);
		close(pipe_proc2_instr[1]);
		close(pipe_proc2_ackn[0]);
		close(pipe_parent_wait[0]);

  	    char *filename;
		int y, result;
		while(1) {
			filename = malloc(8 * sizeof(char));
			read(pipe_proc2_instr[0], filename, 8 * sizeof(char));
			read(pipe_proc2_instr[0], &y, sizeof(int));
			printf("SubProcess2: Processing (add,%s,%d).",filename,y);
			if(strncmp(filename,"stopstop",8) == 0) {
				printf(" Encountered end of instructions command.\n");
				write(pipe_proc2_ackn[1], "d", 1);
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
			write(pipe_proc2_ackn[1], "d", 1);
			memset(instr,0,100);
			free(instr);
			memset(filename,0,8*sizeof(char));
			free(filename);
		}

		printf("%s\n", "SubProcess2 Exiting!!!");
		exit(0);
	} else {
		close(pipe_proc1_instr[0]);
		close(pipe_proc1_ackn[1]);
		close(pipe_proc2_instr[0]);
		close(pipe_proc2_ackn[1]);
		close(pipe_parent_wait[1]);

		file_ptr = fopen("instruction.dat", "r");
		instr = malloc(100 * sizeof(char));
		while( ( fgets(instr,100,file_ptr) ) != NULL ) {
			instr = strtok(instr,"\n");
			char *command = strtok(instr, " ");
			if(strncmp(command,"fac",3) == 0) {
			    int x = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
			    int y = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
				printf("Parent Process: Adding (fac,%d,%d) to SubProcess1 Queue.\n",x,y);
				write(pipe_proc1_instr[1], &x, sizeof(int));
				write(pipe_proc1_instr[1], &y, sizeof(int));
				N1++;
			} else if(strncmp(command,"add",3) == 0) {
			    char *x = strtok(NULL, " ");
			    int y = (int) strtol(strtok(NULL, " "), (char **)NULL, 10);
				printf("Parent Process: Adding (add,%s,%d) to SubProcess2 Queue.\n",x,y);
				write(pipe_proc2_instr[1], x, 8*sizeof(char));
				write(pipe_proc2_instr[1], &y, sizeof(int));
				N2++;
			} else {
				fprintf(stderr,"Invalid command in instruction.dat file: %s %s\n",command,instr);
			}
			memset(instr,0,100);
			free(instr);
			instr = malloc(100 * sizeof(char));
			if(N1 >= N || N2 >= N) {
				char *temp;
				while(N1 >= N) {
					read(pipe_proc1_ackn[0], temp, 1);
					N1--;
				}
				while(N2 >= N) {
					read(pipe_proc2_ackn[0], temp, 1);
					N2--;
				}
			}
		}
		fclose(file_ptr);

		instr = malloc(100 * sizeof(char));
		read(pipe_parent_wait[0],instr,1);
		memset(instr,0,100);
		free(instr);
		printf("%s\n", "Parent Process Exiting!!!");
		exit(0);
	}
  }
}
