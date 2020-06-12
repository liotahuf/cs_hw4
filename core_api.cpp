/* 046267 Computer Architecture - Spring 2020 - HW #4 */

#include "core_api.h"
#include "sim_api.h"


#include <stdio.h>
#include <vector>
#include <queue>


typedef enum {
	READY = 0,
	LOADING =1,     
	STORING =2,     
	DONE =3, 
} status ;

typedef struct myThread {
	tcontext threadContext;
	int threadId;
	int PC;
	int loadCnt;
	int storeCnt;
	status myStatus;

}thread_t;

int blockedRRthreadID;
int blockedCPUthreadID;
std::vector<thread_t> blockedThreadVec;
int instCntBlocked;
int clkCntBloked;

//int instCntBlocked;
//int clkCntBlocked;
//std::queue<thread_t> blockedThreadQReady;
//std::queue<thread_t> blockedThreadQStore;
//std::queue<thread_t> blockedThreadQLoad;
//int lastQin;
//
//
//
int instCntFG;
int clkCntFG;

//helper functions
void updateThreadsCnts(int clckCycles);
int searchReadyThread(int startingIdx);
bool isSimDone();


void CORE_BlockedMT() {
	//initiate variables
	instCntFG = 0;
	clkCntFG = 0;
	
	//initiate therad vector
	blockedThreadVec.push_back({});
	int threadNum = SIM_GetThreadsNum();
	for (int i = 0; i < threadNum; i++)
	{
		blockedThreadVec[i].PC = 0;
		for (int j = 0; j < REGS_COUNT; j++)
		{
			blockedThreadVec[i].threadContext.reg[j] = 0;
		}
		blockedThreadVec[i].threadId = i;
		blockedThreadVec[i].loadCnt = 0;
		blockedThreadVec[i].storeCnt = 0;
		blockedThreadVec[i].myStatus =READY;
			   	
	}

	//initiate RRidx
	blockedCPUthreadID = 0;


	//start running 
	while (!isSimDone())
	{
		if (blockedThreadVec[blockedCPUthreadID].myStatus != READY)
		{
			//search for READY thread
			int  nextThreadID = searchReadyThread(blockedCPUthreadID);
			if (nextThreadID == -1)
			{
				//update clock cycle
				clkCntFG++;
				//update not ready instrctutions that a cycle has passed
				updateThreadsCnts(1);
				continue;

			}
			else
			{
				//do context switch
				//update clock cycle
				clkCntFG+= SIM_GetSwitchCycles();
				//update not ready instrctutions that a cycle has passed
				updateThreadsCnts(SIM_GetSwitchCycles());
				//update CPU thread iD
				blockedCPUthreadID = nextThreadID;
			}
		}
		
		
		//do instruction 
		
		Instruction* currInstr;
		SIM_MemInstRead(blockedThreadVec[blockedCPUthreadID].PC, currInstr, blockedThreadVec[blockedCPUthreadID].threadId);

		switch (currInstr->opcode)
		{
		case CMD_LOAD: // dst <- Mem[src1 + src2]  (src2 may be an immediate)
			//read from memory and load into register accordinto to instr
				
			//first,get load ints src
			int address = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src1_index];
			if (currInstr->isSrc2Imm) {
				address += currInstr->src2_index_imm;
			}
			else
			{
				address += blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src2_index_imm];
			}
				
				
			//load instrc dst
			int *dst = &(blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->dst_index]);
			//load into register
			SIM_MemDataRead(address, dst);
			//update status
			blockedThreadVec[blockedCPUthreadID].myStatus = LOADING;

			break;
		case CMD_STORE: // Mem[dst + src2] <- src1  (src2 may be an immediate)
			//read from src register and store it into memory at address

			//first, calculate dst +scr2 a
			int address = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->dst_index];
			if (currInstr->isSrc2Imm) {
				address += currInstr->src2_index_imm;
			}
			else
			{
				address += blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src2_index_imm];
			}

			//get the value of regs[src1]

			int value = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src1_index];

			//store value into Mem[address]
			SIM_MemDataWrite(address, value);
			blockedThreadVec[blockedCPUthreadID].myStatus = STORING;

			break;

		case CMD_HALT:
			blockedThreadVec[blockedCPUthreadID].myStatus = DONE;
			break;

		case CMD_ADD:		// dst <- src1 + src2

			int scr1 = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src1_index];
			int scr2 = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src2_index_imm];
			blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->dst_index] = scr1 + scr2;
			break;

		case CMD_SUB:		// dst <- src1 - src2
			int scr1 = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src1_index];
			int scr2 = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src2_index_imm];
			blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->dst_index] = scr1 - scr2;
			break;
	
		case CMD_ADDI:		// dst <- src1 + imm
			int scr1 = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src1_index];
			int scr2 = currInstr->src2_index_imm;
			blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->dst_index] = scr1 + scr2;
			break;

		case CMD_SUBI:		// dst <- src1 - imm
			int scr1 = blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->src1_index];
			int scr2 = currInstr->src2_index_imm;
			blockedThreadVec[blockedCPUthreadID].threadContext.reg[currInstr->dst_index] = scr1 - scr2;
			break;
			
		default:

			break;
		}

		//update clock cycle
		clkCntFG++;
		//update instrc cnt
		instCntFG++;
		//update PC of thread
		blockedThreadVec[blockedCPUthreadID].PC += 4;
	}	

}

void CORE_FinegrainedMT() {

}

double CORE_BlockedMT_CPI(){
	
	return ((double)clkCntBloked/instCntBlocked);
}

double CORE_FinegrainedMT_CPI(){
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {

	*context = blockedThreadVec[threadid].threadContext;
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}


void updateThreadsCnts(int clckCycles)
{
	int threadNum = SIM_GetThreadsNum();
	for (int i = 0; i < threadNum; i++)
	{
		if (blockedThreadVec[i].myStatus == LOADING)
		{
			blockedThreadVec[i].loadCnt += clckCycles;
			
			if (blockedThreadVec[i].loadCnt >= SIM_GetLoadLat())
			{
				blockedThreadVec[i].myStatus = READY;
				blockedThreadVec[i].loadCnt = 0;
			}
		}
		else if (blockedThreadVec[i].myStatus == STORING)
		{
			blockedThreadVec[i].storeCnt += clckCycles;

			if (blockedThreadVec[i].storeCnt >= SIM_GetStoreLat())
			{
				blockedThreadVec[i].myStatus = READY;
				blockedThreadVec[i].storeCnt = 0;
			}
		}
	}

}

int searchReadyThread(int startingIdx)
{
	if (blockedThreadVec[startingIdx].myStatus == READY)
	{
		return startingIdx;
	}
	int threadsNum = SIM_GetThreadsNum();
	int nextIdx = (startingIdx + 1)%(threadsNum);
	for (; nextIdx != startingIdx; nextIdx = (nextIdx + 1) % (threadsNum))
	{
		if (blockedThreadVec[nextIdx].myStatus == READY)
		{
			return nextIdx;
		}
	}

	return -1;
}

bool isSimDone()
{
	for (int i = 0; i < SIM_GetThreadsNum(); i++)
	{
		if (blockedThreadVec[i].myStatus != DONE)
			return false;
	}
	return true;
}