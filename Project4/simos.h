#include <semaphore.h>
#include <math.h>
//================= general definitions =========================

#define Debug 1

typedef unsigned *genericPtr;
          // when passing pointers externally, use genericPtr
          // to avoid the necessity of exposing internal structures


//============ sytem configuration parameters ===================
int Observe;  // whether to print more information for better observation
              // = 0 turn off observation mode; = 1 turn on

int pageSize, memSize, swapSize, OSmemSize;
                   // sizes related to memory and memory management
int periodAgeScan; // the period for scanning and shifting the age vectors
                   // defined in # instruction-cycles

int cpuQuantum;    // time quantum, defined in # instruction-cycles
int idleQuantum;   // time quantum for the idle process

int spoolPsize;    // spool space size for each process



//================= CPU related definitions ======================


struct
{ int Pid;
  int PC;
  float AC;
  float MBR;
  int IRopcode;
  int IRoperand;
  int* Mbase;
  int MDbase;
  int Mbound;
  char *spoolPtr; 
  int spoolPos; 
  int exeStatus;
  unsigned interruptV;
  int numCycles;  // this is a global register, not for each process
} CPU;


// define interrupt set bit for interruptV in CPU structure
// 1 means bit 0, 4 means bit 2, ...

#define tqInterrupt 1      // for time quantum
#define ageInterrupt 2     // for age scan
#define doneWaitInterrupt 4  // for any IO completion, including page fault
        // before setting doneWait, caller should add the pid to doneWait list


// define exeStatus in CPU structure
#define eRun 1
#define eReady 2
#define ePFault 3
#define eWait 4
#define eEnd 0
#define eError -1


// definition related to numCycles
#define maxCPUcycles 1024*1024*1024 // = 2^30

//IO thread declaration
pthread_t IOThread;
volatile sem_t IOsem;
volatile sem_t IOQsem;

// cpu function definitions

void initialize_cpu ();
void dump_registers ();
void cpu_execution ();

void set_interrupt (unsigned bit);



//=============== process related definitions ====================

typedef struct
{ int Pid;
  int PC;
  float AC;
  int* Mbase;
  int MDbase;
  int Mbound;
  int* Mswapbase;
  char *spoolPtr;
  int spoolPos;
  int exeStatus;
  int numInstr;
  int numStaticData;
  int numData;
      // numData is not in use anywhere
      // it is useful if there is dynamic space used during run time
      // but here we do not consider it
  int timeUsed;
} typePCB;

#define maxProcess 1024
typePCB *PCB[maxProcess];
  // the system can have #maxProcess processes,
  // first one is OS, so, pid of any user process starts from 1
  // each process get a PCB, allocate PCB space upon process creation

#define osPid 0
#define idlePid 1
int currentPid;    // user pid should start from 2, pid=0/1 are defined above


// define process manipulation functions

void dump_PCB (int pid);
void dump_ready_queue ();

void insert_doneWait_process (int pid);
void dump_doneWait_list ();

void initialize_process ();
void submit_process (char* fname);
void execute_process ();



//=============== memory related definitions ====================

#define mNormal 0
#define mError -1
#define mPFault -2

#define totalFrameCnt (int)floor(memSize/pageSize)
#define OSFrameCnt (int)ceil((float)OSmemSize/(float)pageSize)
#define totalSwapFrameCnt (int)floor(swapSize/pageSize)

typedef struct {
	int mInstr;
	float mData;
} mType;
mType *memory;
mType *swapMemory;

typedef struct {
	char isFree;
	int pid;
	int pgNum;
	int oldSwapFrameNum;
	char isDirty;
	int ageVector;
	mType *memory;
} mFrame;
mFrame	*frameTable;

typedef struct {
	char isFree;
	mType *memory;
} mSwapFrame;
mSwapFrame	*swapFrameTable;

// memory related function definitions

int get_data (int offset);
int put_data (int offset);
int get_instruction (int offset);

int load_instruction (int pid, int offset, int opcode, int operand);
int load_data (int pid, int offset, float data);
void copy_first_frame_into_memory(int pid);

void memory_agescan ();
void init_pagefault_handler (int pid);

int allocate_memory (int pid, int msize, int numinstr);
int free_memory (int pid);

void initialize_memory ();
void dump_all_memory ();
void dump_memory (int pid);
void dump_all_swap_space();
void dump_swap_space(int pid);


//=============== timer related definitions ====================

#define oneTimeTimer 0

// define the action codes for timer
#define actTQinterrupt 1
#define actAgeInterrupt 2
#define actReadyInterrupt 3
#define actNull 0


// define the timer functions 

void dump_events ();

void initialize_timer ();
genericPtr add_timer (int time, int pid, int action, int recurperiod);
void check_timer ();
void deactivate_timer (genericPtr castedevent);



//=============== spooler related definitions ====================

#define spNormal 0
#define spError -1

#define prNormal 0
#define prError -1


// define the spooler functions 

int spool (char* str);

void allocate_spool (int pid);
void free_spool (int pid);
void print_spool (int pid);

void dump_spool (int pid);

int printer (int pid, int status, char* prstr, int len);



//=============== I/O related definitions ====================

typedef struct {
	int pid;
	char reqType;	//i=swapIn, o=swapInOut
	int memFrameNum;
	int outSwapFrameNum;
	int inSwapFrameNum;
	struct mIORequestNode* next;
} mIORequestNode;
mIORequestNode *IOReqHead;
mIORequestNode *IOReqTail;

void insert_IO (mIORequestNode* node);
int process_IO();
void IO_main_method();
void dump_IO();
