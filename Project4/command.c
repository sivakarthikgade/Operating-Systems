#include <stdio.h>
#include <pthread.h>
#include "simos.h"


void initialize_system ()
{ FILE *fconfig;

  fconfig = fopen ("config.sys", "r");
  fscanf (fconfig, "%d\n", &Observe);
  fscanf (fconfig, "%d %d\n", &cpuQuantum, &idleQuantum);
  fscanf (fconfig, "%d %d %d %d\n", &pageSize, &memSize, &swapSize, &OSmemSize);
  fscanf (fconfig, "%d\n", &periodAgeScan);
  fscanf (fconfig, "%d\n", &spoolPsize);
  fclose (fconfig);

  initialize_cpu ();
  initialize_timer ();
  initialize_memory ();
  initialize_process ();

  sem_init((sem_t *) &IOQsem, 0, 1);
  sem_init((sem_t *) &IOsem, 0, 0);
  pthread_create(&IOThread, NULL, IO_main_method, (void *) NULL);
}


void process_command ()
{ char action;
  char fname[100];
  int pid, time, ret;
  mIORequestNode* temp;

  printf ("command> ");
  scanf ("%c", &action);
  while (action != 'T')
  { switch (action)
    { case 's':   // submit
        scanf ("%s", &fname);
        if (Debug) printf ("File name: %s is submitted\n", fname);
        submit_process (fname);
        break;
      case 'x':  // execute
        execute_process ();
        break;
      case 'r':  // dump register
        dump_registers ();
        break;
      case 'q':  // dump ready queue and list of processes completed IO
        dump_ready_queue ();
        dump_doneWait_list ();
        break;
      case 'p':   // dump PCB
        printf ("PCB Dump Starts: Checks from 0 to %d\n", currentPid);
        for (pid=1; pid<currentPid; pid++)
          if (PCB[pid] != NULL) dump_PCB (pid);
        break;
      case 'e':   // dump events in timer
        dump_events ();
        break;
      case 'm':   // dump Memory
        dump_all_memory ();
        break;
      case 'w':   // dump Swap space
     	dump_all_swap_space();
    	break;
      case 'i':   // dump IO requests
     	dump_IO();
    	break;
      case 'l':   // dump Spool
        for (pid=1; pid<currentPid; pid++)
          if (PCB[pid] != NULL) dump_spool (pid);
        break;
      case 'T':  // Terminate, do nothing, terminate in while loop
   		temp = malloc(sizeof(mIORequestNode));
   		temp->pid = -1;
   		insert_IO(temp);
    	sem_post((sem_t *) &IOsem);
		pthread_join(IOThread, NULL);
		sem_destroy((sem_t *) &IOsem);
		sem_destroy((sem_t *) &IOQsem);
        break;
      default: 
        printf ("Incorrect command!!!\n");
    }
    printf ("\ncommand> ");
    scanf ("\n%c", &action);
    if (Debug) printf ("Next command is %c\n", action);
  }
}


void main ()
{
  initialize_system ();
  process_command ();
}
