#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simos.h"

/*
 * memory_new.c
 *
 *  Created on: Apr 12, 2014
 *      Author: sivakarthik
 */
#define opcodeShift 24
#define operandMask 0x00ffffff

void initialize_memory () {
	int i;
	memory = malloc(memSize * sizeof(mType));
	frameTable = malloc(totalFrameCnt * sizeof(mFrame));
	for(i = 0; i < totalFrameCnt; i++) {
		if(i < OSFrameCnt) {
			frameTable[i].isFree = 'n';
		} else {
			frameTable[i].isFree = 'y';
		}
		frameTable[i].pid = -1;
		frameTable[i].pgNum = -1;
		frameTable[i].oldSwapFrameNum = -1;
		frameTable[i].isDirty = 'n';
		frameTable[i].ageVector = 0;
		frameTable[i].memory = &memory[i * pageSize];
		memset(frameTable[i].memory, 0, pageSize*sizeof(mType));
	}

	//swap space initialization
	swapMemory = malloc(swapSize * sizeof(mType));
	swapFrameTable = malloc(totalSwapFrameCnt * sizeof(mSwapFrame));
	for(i = 0; i < totalSwapFrameCnt; i++) {
		swapFrameTable[i].isFree = 'y';
		swapFrameTable[i].memory = &swapMemory[i * pageSize];
		memset(swapFrameTable[i].memory, 0, pageSize*sizeof(mType));
	}

	add_timer (periodAgeScan, osPid, actAgeInterrupt, periodAgeScan);
	// in demand paging, some more initialization is probably needed
}

int allocate_memory (int pid, int msize, int numinstr) {
	if (pid >= maxProcess) {
		printf ("memory.allocate_memory: Invalid pid: %d\n", pid);
		return(mError);
	} else if ((int)ceil((float)msize/(float)pageSize) > totalSwapFrameCnt) {
		printf ("memory.allocate_memory: Invalid memory size %d for process %d. Exceeds OS swap size.\n", msize, pid);
	    return(mError);
	}

	int reqSwapPgCnt = (int)ceil((float)msize/(float)pageSize);
	if(reqSwapPgCnt > 0) {
		int availableSwapPgCnt = 0;
		int i = 0;
		for(i = 0; i < totalSwapFrameCnt; i++) {
			if(swapFrameTable[i].isFree == 'y') {
				availableSwapPgCnt++;
			}
		}
		if((totalFrameCnt - OSFrameCnt) <= 0) {
			availableSwapPgCnt = 0;
		}
		if(availableSwapPgCnt >= reqSwapPgCnt) {
			int* swapPageTable = malloc(reqSwapPgCnt * sizeof(int));
			int* pageTable = malloc(reqSwapPgCnt * sizeof(int));
			int j = 0;
			for(i = 0; i < totalSwapFrameCnt; i++) {
				if(swapFrameTable[i].isFree == 'y') {
					swapFrameTable[i].isFree = 'n';
					swapPageTable[j] = i;
					pageTable[j] = -1;
					j++;
				}
				if(j == reqSwapPgCnt) {
					break;
				}
			}
			pageTable[0] = getFreeMemoryFrame(pid, 0, 1);
			PCB[pid]->Mbase = pageTable;
			PCB[pid]->Mbound = 0;
			PCB[pid]->MDbase = 0;
			PCB[pid]->Mswapbase = swapPageTable;
			return mNormal;
		} else {
			printf ("memory.allocate_memory: Can't load process %d. Not enough free frames available!\n",pid);
			return (mError);
		}
	} else {
		return mNormal;
	}
}

int getFreeMemoryFrame(int pid, int pgNum, int isSaveDirtyData) {
	int i, frameNum = -1;
	for(i = OSFrameCnt; i < totalFrameCnt; i++) {
		if(frameTable[i].isFree == 'y') {
			frameNum = i;
		}
		if(frameNum != -1) {
			break;
		}
	}
	if(frameNum == -1) {
		frameNum = freeMemoryFrame(isSaveDirtyData);
	}

	frameTable[frameNum].isFree = 'n';
	frameTable[frameNum].isDirty = 'n';
	frameTable[frameNum].ageVector = 128;
	frameTable[frameNum].pid = pid;
	frameTable[frameNum].pgNum = pgNum;

	return frameNum;
}

int freeMemoryFrame(int isSaveDirtyData) {
	int i, frameNum = -1;
	int age = 256;

	for(i = OSFrameCnt; i < totalFrameCnt; i++) {
		if(frameTable[i].isFree == 'y') {
			frameNum = i;
			break;
		} else {
			if(frameTable[i].ageVector < age) {
				frameNum = i;
				age = frameTable[i].ageVector;
				if(age == 0) {
					break;
				}
			}
		}
	}

	printf("Freeing frame %d which belonged to process %d page %d, to serve new frame requirement.\n", frameNum, frameTable[frameNum].pid, frameTable[frameNum].pgNum);
	if(frameTable[frameNum].isDirty == 'y') {
		if(isSaveDirtyData == 1) {
			for(i = 0; i < pageSize; i++) {
				swapFrameTable[PCB[frameTable[frameNum].pid]->Mswapbase[frameTable[frameNum].pgNum]].memory[i].mInstr = frameTable[frameNum].memory[i].mInstr;
				swapFrameTable[PCB[frameTable[frameNum].pid]->Mswapbase[frameTable[frameNum].pgNum]].memory[i].mData = frameTable[frameNum].memory[i].mData;
			}
			memset(frameTable[frameNum].memory, 0, pageSize*sizeof(mType));
			frameTable[frameNum].oldSwapFrameNum = -1;
		} else {
			frameTable[frameNum].oldSwapFrameNum = PCB[frameTable[frameNum].pid]->Mswapbase[frameTable[frameNum].pgNum];
		}
	}
	PCB[frameTable[frameNum].pid]->Mbase[frameTable[frameNum].pgNum] = -1;

	frameTable[frameNum].isFree = 'y';
	frameTable[frameNum].isDirty = 'n';
	frameTable[frameNum].ageVector = 0;
	frameTable[frameNum].pid = -1;
	frameTable[frameNum].pgNum = -1;

	return frameNum;
}

int free_memory (int pid) {
	int i = 0;
	int totalPageCnt = (int)ceil((float)(PCB[pid]->numInstr + PCB[pid]->numStaticData)/(float)pageSize);
	for(i = 0; i < totalPageCnt; i++) {
		if(PCB[pid]->Mbase[i] != -1) {
			memset(frameTable[PCB[pid]->Mbase[i]].memory, 0, pageSize*sizeof(mType));
			frameTable[PCB[pid]->Mbase[i]].isFree = 'y';
			frameTable[PCB[pid]->Mbase[i]].isDirty = 'n';
			frameTable[PCB[pid]->Mbase[i]].ageVector = 0;
			frameTable[PCB[pid]->Mbase[i]].pid = -1;
			frameTable[PCB[pid]->Mbase[i]].pgNum = -1;
			frameTable[PCB[pid]->Mbase[i]].oldSwapFrameNum = -1;
		}

		memset(swapFrameTable[PCB[pid]->Mswapbase[i]].memory, 0, pageSize*sizeof(mType));
		swapFrameTable[PCB[pid]->Mswapbase[i]].isFree = 'y';
	}
	free(PCB[pid]->Mbase);
	free(PCB[pid]->Mswapbase);
	return mNormal;
}

int check_load_address (int pid, int offset, char type) {
	if((type == 'i') && (offset >= PCB[pid]->numInstr)) {
		printf ("memory.check_load_address: Process %d accesses offset %d. Instruction offset limit exceeded.!\n", pid, offset);
		return (mError);
	}
	if((type == 'd') && (offset >= PCB[pid]->numStaticData)) {
		printf ("memory.check_load_address: Process %d accesses offset %d. Outside address space.!\n", pid, offset);
		return (mError);
	}
	return (mNormal);
}

int load_instruction (int pid, int offset, int opcode, int operand) {
	if (check_load_address (pid, offset, 'i') == mError)
		return (mError);
	else {
		opcode = opcode << opcodeShift;
		operand = operand & operandMask;
		swapFrameTable[PCB[pid]->Mswapbase[(int)floor(offset/pageSize)]].memory[offset%pageSize].mInstr = opcode | operand;
		return (mNormal);
	}
}

int load_data (int pid, int offset, float data) {
	if (check_load_address (pid, offset, 'd') == mError)
		return (mError);
	else {
		swapFrameTable[PCB[pid]->Mswapbase[(int)floor((PCB[pid]->numInstr+offset)/pageSize)]].memory[(PCB[pid]->numInstr+offset)%pageSize].mData = data;
		return (mNormal);
	}
}

void copy_first_frame_into_memory(int pid) {
	int i;
	for(i = 0; i < pageSize; i++) {
		frameTable[PCB[pid]->Mbase[0]].memory[i].mInstr = swapFrameTable[PCB[pid]->Mswapbase[0]].memory[i].mInstr;
		frameTable[PCB[pid]->Mbase[0]].memory[i].mData = swapFrameTable[PCB[pid]->Mswapbase[0]].memory[i].mData;
	}
}

int check_address (int offset, char type) {
	if((type == 'i') && (offset >= PCB[CPU.Pid]->numInstr)) {
		printf ("memory.check_address: Process %d accesses offset %d. Instruction offset limit exceeded.!\n", CPU.Pid, offset);
		return (mError);
	}
	if((type == 'd') && (offset >= PCB[CPU.Pid]->numStaticData)) {
		printf ("memory.check_address: Process %d accesses offset %d. Outside address space.!\n", CPU.Pid, offset);
		return (mError);
	}
//	if (Debug) {
//		if(type == 'd') {
//			offset = offset + PCB[CPU.Pid]->numInstr;
//		}
//		printf ("content = %x, %.2f\n", frameTable[PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)]].memory[offset%pageSize].mInstr, frameTable[PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)]].memory[offset%pageSize].mData);
//	}
	return (mNormal);
}

int compute_address(int offset) {
	int address;
	if(PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)] == -1) {
		//Get a free frame & load the respective contents from swap space.
		printf("memory.compute_address: Page Fault Occurred. Process %d, accessing page %d.\n",CPU.Pid,(int)floor(offset/pageSize));
		if(CPU.Pid == 1) {
			int frameNum = getFreeMemoryFrame(CPU.Pid, (int)floor(offset/pageSize), 1);
			printf("memory.compute_address: Frame number alloted %d.\n",frameNum);
			PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)] = frameNum;
			//write data from swap space to memory frame here itself in case of idle process.
			int i;
			for(i = 0; i < pageSize; i++) {
				frameTable[frameNum].memory[i].mInstr = swapFrameTable[PCB[CPU.Pid]->Mswapbase[(int)floor(offset/pageSize)]].memory[i].mInstr;
				frameTable[frameNum].memory[i].mData = swapFrameTable[PCB[CPU.Pid]->Mswapbase[(int)floor(offset/pageSize)]].memory[i].mData;
			}
			address = (PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)])*pageSize + (offset%pageSize);
		} else {
			int frameNum = getFreeMemoryFrame(CPU.Pid, (int)floor(offset/pageSize), 0);
			printf("memory.compute_address: Frame number alloted %d.\n",frameNum);
			PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)] = frameNum;
			mIORequestNode* node = malloc(sizeof(mIORequestNode));
			node->pid = CPU.Pid;
			if(frameTable[frameNum].oldSwapFrameNum == -1) {
				node->reqType = 'i';
				node->outSwapFrameNum = -1;
			} else {
				node->reqType = 'o';
				node->outSwapFrameNum = frameTable[frameNum].oldSwapFrameNum;
				frameTable[frameNum].oldSwapFrameNum = -1;
			}
			node->memFrameNum = frameNum;
			node->inSwapFrameNum = PCB[CPU.Pid]->Mswapbase[(int)floor(offset/pageSize)];
			node->next = NULL;
			insert_IO(node);
			address = mPFault;
		}
	} else {
		address = (PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)])*pageSize + (offset%pageSize);
	}
	if(Observe) {
		printf("memory.compute_address: pid=%d, input_offset=%d, output_address=%d\n", CPU.Pid, offset, (PCB[CPU.Pid]->Mbase[(int)floor(offset/pageSize)])*pageSize + (offset%pageSize));
	}
	return address;
}

int get_instruction (int offset) {
	int instr, address;
	if (check_address (offset, 'i') == mError) {
		return (mError);
	} else {
		address = compute_address(offset);
		if(address == mPFault) {
			return mPFault;
		}
		if(Observe) {
			printf("memory.get_instruction: process %d accessed page %d (frame %d). Params before change: AgeVector - %d\n", CPU.Pid, (int)floor(offset/pageSize), (int)floor(address/pageSize), frameTable[(int)floor(address/pageSize)].ageVector);
		}
		frameTable[(int)floor(address/pageSize)].ageVector = 128 | frameTable[(int)floor(address/pageSize)].ageVector;
		if(Observe) {
			printf("memory.get_instruction: process %d accessed page %d (frame %d). Params after change: AgeVector - %d\n", CPU.Pid, (int)floor(offset/pageSize), (int)floor(address/pageSize), frameTable[(int)floor(address/pageSize)].ageVector);
		}
		instr = memory[address].mInstr;
		CPU.IRopcode = instr >> opcodeShift;
		CPU.IRoperand = instr & operandMask;
		return (mNormal);
	}
}

int get_data (int offset) {
	int address;
	if (check_address (offset, 'd') == mError) {
		return (mError);
	} else {
		address = compute_address(PCB[CPU.Pid]->numInstr+offset);
		if(address == mPFault) {
			return mPFault;
		}
		if(Observe) {
			printf("memory.get_data: process %d accessed page %d (frame %d). Params before change: AgeVector - %d\n", CPU.Pid, (int)floor(offset/pageSize), (int)floor(address/pageSize), frameTable[(int)floor(address/pageSize)].ageVector);
		}
		frameTable[(int)floor(address/pageSize)].ageVector = 128 | frameTable[(int)floor(address/pageSize)].ageVector;
		if(Observe) {
			printf("memory.get_data: process %d accessed page %d (frame %d). Params after change: AgeVector - %d\n", CPU.Pid, (int)floor(offset/pageSize), (int)floor(address/pageSize), frameTable[(int)floor(address/pageSize)].ageVector);
		}
		CPU.MBR = memory[address].mData;
		return (mNormal);
	}
}

int put_data (int offset) {
	int address;
	if (check_address (offset, 'd') == mError) {
		return (mError);
	} else {
		address = compute_address(PCB[CPU.Pid]->numInstr+offset);
		if(address == mPFault) {
			return mPFault;
		}
		if(Observe) {
			printf("memory.put_data: process %d accessed page %d (frame %d). Params before change: AgeVector - %d; isDirty - %c\n", CPU.Pid, (int)floor(offset/pageSize), (int)floor(address/pageSize)
					, frameTable[(int)floor(address/pageSize)].ageVector, frameTable[(int)floor(address/pageSize)].isDirty);
		}
		frameTable[(int)floor(address/pageSize)].ageVector = 128 | frameTable[(int)floor(address/pageSize)].ageVector;
		frameTable[(int)floor(address/pageSize)].isDirty = 'y';
		if(Observe) {
			printf("memory.put_data: process %d accessed page %d (frame %d). Params after change: AgeVector - %d; isDirty - %c\n", CPU.Pid, (int)floor(offset/pageSize), (int)floor(address/pageSize)
					, frameTable[(int)floor(address/pageSize)].ageVector, frameTable[(int)floor(address/pageSize)].isDirty);
		}
		memory[address].mData = CPU.AC;
		return (mNormal);
	}
}

void memory_agescan () {
	printf ("Scan and update age vectors for memory pages.\n");
	int i, j;
	for(i = OSFrameCnt; i < totalFrameCnt; i++) {
		if(frameTable[i].isFree == 'n') {
			frameTable[i].ageVector = frameTable[i].ageVector >> 1;
//			if(frameTable[i].ageVector == 0) {
//				if(frameTable[i].isDirty == 'y') {
//					for(j = 0; j < pageSize; j++) {
//						swapFrameTable[PCB[frameTable[i].pid]->Mswapbase[frameTable[i].pgNum]].memory[j].mInstr = frameTable[i].memory[j].mInstr;
//						swapFrameTable[PCB[frameTable[i].pid]->Mswapbase[frameTable[i].pgNum]].memory[j].mData = frameTable[i].memory[j].mData;
//					}
//				}
//				PCB[frameTable[i].pid]->Mbase[frameTable[i].pgNum] = -1;
//				memset(frameTable[i].memory, 0, pageSize*sizeof(mType));
//				frameTable[i].isFree = 'y';
//				frameTable[i].isDirty = 'n';
//				frameTable[i].ageVector = 0;
//				frameTable[i].pid = -1;
//				frameTable[i].pgNum = -1;
//			}
		}
	}
}

void init_pagefault_handler (int pid) {
	printf ("Page fault handler is being activated.\n");
	sem_post((sem_t *) &IOsem);
}

void pagefault_complete (pid) {
	insert_doneWait_process (pid);
	set_interrupt (doneWaitInterrupt);
}

void dump_all_memory () {
	int i, pid, start, end;

	start = -1;
	end = -1;
	printf("************ List of Free Memory Frames ************\n");
	for(i = 0; i < totalFrameCnt; i++) {
		if(frameTable[i].isFree == 'y') {
			if(start == -1) {
				start = i;
				end = i;
			} else {
				end = i;
			}
		} else {
			if(start == -1) {
				continue;
			} else {
				if(start == end) {
					printf("%d,",start);
				} else {
					printf("%d-%d,",start,end);
				}
				start = -1;
				end = -1;
			}
		}
	}
	if(start != -1) {
		if(start == end) {
			printf("%d,",start);
		} else {
			printf("%d-%d,",start,end);
		}
	}
	printf("\n");

	for (pid=1; pid<currentPid; pid++) {
      if (PCB[pid] != NULL) dump_memory (pid);
	}

}

void dump_memory (int pid) {
	int i, j, offset, pgCnt;
	pgCnt = (int)ceil((float)(PCB[pid]->numInstr + PCB[pid]->numStaticData)/(float)pageSize);

	printf ("PROCESS %d MEMORY DUMP STARTS:\n", pid);
	printf ("************ Page Table Memory Dump for Process %d ************\n", pid);
	for(i = 0; i < pgCnt; i++)
		printf("(%d,%d),",i,PCB[pid]->Mbase[i]);
	printf("\n");

	printf ("************ Instruction Memory Dump for Process %d ************\n", pid);
	offset = 0;
	for(i = 0; i < pgCnt; i++) {
		printf("(Page,Frame): (%d,%d); AgeVector: %d; isDirty: %c;\n",i,PCB[pid]->Mbase[i]
						, PCB[pid]->Mbase[i] == -1 ? -1 : frameTable[PCB[pid]->Mbase[i]].ageVector
						, PCB[pid]->Mbase[i] == -1 ? '-' : frameTable[PCB[pid]->Mbase[i]].isDirty);
		if(PCB[pid]->Mbase[i] == -1) {
			offset = offset + pageSize;
			continue;
		}
		for(j = 0; j < pageSize; j++) {
			if(offset == (PCB[pid]->numInstr + PCB[pid]->numStaticData)) {
				break;
			}
			if(offset == PCB[pid]->numInstr) {
				printf ("\n************ Data Memory Dump for Process %d ************\n", pid);
			}
			if(offset < PCB[pid]->numInstr) {
				printf("%x ", frameTable[PCB[pid]->Mbase[i]].memory[j].mInstr);
			} else {
				printf("%.2f ", frameTable[PCB[pid]->Mbase[i]].memory[j].mData);
			}
			offset++;
		}
		printf("\n");
	}
}

void dump_all_swap_space() {
	int i, pid, start, end;

	start = -1;
	end = -1;
	printf("************ List of Free Swap Space Frames ************\n");
	for(i = 0; i < totalSwapFrameCnt; i++) {
		if(swapFrameTable[i].isFree == 'y') {
			if(start == -1) {
				start = i;
				end = i;
			} else {
				end = i;
			}
		} else {
			if(start == -1) {
				continue;
			} else {
				if(start == end) {
					printf("%d,",start);
				} else {
					printf("%d-%d,",start,end);
				}
				start = -1;
				end = -1;
			}
		}
	}
	if(start != -1) {
		if(start == end) {
			printf("%d,",start);
		} else {
			printf("%d-%d,",start,end);
		}
	}
	printf("\n");

	for (pid=1; pid<currentPid; pid++) {
      if (PCB[pid] != NULL) dump_swap_space (pid);
	}
}

void dump_swap_space(int pid) {
	int i, j, offset, pgCnt;
	pgCnt = (int)ceil((float)(PCB[pid]->numInstr + PCB[pid]->numStaticData)/(float)pageSize);

	printf ("PROCESS %d SWAP SPACE DUMP STARTS:\n", pid);
	printf ("************ Page Table Swap Space Dump for Process %d ************\n", pid);
	for(i = 0; i < pgCnt; i++)
		printf("(%d,%d),",i,PCB[pid]->Mswapbase[i]);
	printf("\n");

	printf ("************ Instruction Swap Space Dump for Process %d ************\n", pid);
	offset = 0;
	for(i = 0; i < pgCnt; i++) {
		printf("(Page,Frame): (%d,%d)\n",i,PCB[pid]->Mswapbase[i]);
		if(PCB[pid]->Mswapbase[i] == -1) {
			offset = offset + pageSize;
			continue;
		}
		for(j = 0; j < pageSize; j++) {
			if(offset == (PCB[pid]->numInstr + PCB[pid]->numStaticData)) {
				break;
			}
			if(offset == PCB[pid]->numInstr) {
				printf ("\n************ Data Swap Space Dump for Process %d ************\n", pid);
			}
			if(offset < PCB[pid]->numInstr) {
				printf("%x ", swapFrameTable[PCB[pid]->Mswapbase[i]].memory[j].mInstr);
			} else {
				printf("%.2f ", swapFrameTable[PCB[pid]->Mswapbase[i]].memory[j].mData);
			}
			offset++;
		}
		printf("\n");
	}
}

void insert_IO (mIORequestNode* node) {
	sem_wait((sem_t *) &IOQsem);
	if (IOReqTail == NULL) { // IOReqHead would be NULL also
		IOReqTail = node;
		IOReqHead = node;
	} else {// insert to tail
		IOReqTail->next = node;
		IOReqTail = node;
	}
	sem_post((sem_t *) &IOQsem);
}

int process_IO() {
	int retVal = 0;
	mIORequestNode *temp;
	sem_wait((sem_t *) &IOQsem);
	while (IOReqHead != NULL) {
		temp = IOReqHead;
		retVal = temp->pid;
		if(retVal == -1) {
			break;
		}
		if(Observe) {
			printf("IO Request being processed: pid=%d, reqType=%s, memFrameNum=%d, swapInFrameNum=%d, swapOutFrameNum=%d.\n", temp->pid, (temp->reqType == 'i' ? "swapIn" : "swapInOut"), temp->memFrameNum, temp->inSwapFrameNum, temp->outSwapFrameNum);
		}
		//Logic to copy paste (swapout, swapin)
		int i;
		if(temp->reqType == 'o') {
			for(i = 0; i < pageSize; i++) {
				swapFrameTable[temp->outSwapFrameNum].memory[i].mInstr = frameTable[temp->memFrameNum].memory[i].mInstr;
				swapFrameTable[temp->outSwapFrameNum].memory[i].mData = frameTable[temp->memFrameNum].memory[i].mData;
			}
		}
		memset(frameTable[temp->memFrameNum].memory, 0, pageSize*sizeof(mType));
		for(i = 0; i < pageSize; i++) {
			frameTable[temp->memFrameNum].memory[i].mInstr = swapFrameTable[temp->inSwapFrameNum].memory[i].mInstr;
			frameTable[temp->memFrameNum].memory[i].mData = swapFrameTable[temp->inSwapFrameNum].memory[i].mData;
		}
		//Logic to call pagefault processing complete
		pagefault_complete(temp->pid);
		IOReqHead = (mIORequestNode *)temp->next;
		memset(temp, 0, sizeof(mIORequestNode));
		free (temp);
	}
	IOReqTail = NULL;
	sem_post((sem_t *) &IOQsem);
	return retVal;
}

void IO_main_method() {
	int retVal;
	while(1 == 1) {
		sem_wait((sem_t *) &IOsem);
		retVal = process_IO();
		if(retVal == -1) {
			break;
		}
	}
}

void dump_IO() {
	printf ("************ IO Request Dump ************\n");
	mIORequestNode *temp = IOReqHead;
	while((temp != NULL) && (temp->pid != -1)) {
		printf("IO Request: pid=%d, reqType=%s, memFrameNum=%d, swapInFrameNum=%d, swapOutFrameNum=%d.\n", temp->pid, (temp->reqType == 'i' ? "swapIn" : "swapInOut"), temp->memFrameNum, temp->inSwapFrameNum, temp->outSwapFrameNum);
		temp = temp->next;
	}
}
