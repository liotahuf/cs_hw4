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
			blockedThreadVec[i].threadContext[j] = 0;
		}
		blockedThreadVec[i].threadId = i;
		blockedThreadVec[i].loadCnt = 0;
		blockedThreadVec[i].storeCnt = 0;
		blockedThreadVec[i].myStatus =READY;
			   	
	}

	bool simDone =false;

	//initiate RRidx
	blockedRRthreadID = 0;
	blockedCPUthreadID = 0;


	//start running 
	while (!simDone)
	{
		if (blockedThreadVec[blockedRRthreadID].myStatus == READY)
		{
			blockedCPUthreadID = blockedRRthreadID;
		}
		//do instruction 


	}
	
	

}

void CORE_FinegrainedMT() {

}

double CORE_BlockedMT_CPI(){
	return 0;
}

double CORE_FinegrainedMT_CPI(){
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
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

		}
	}

}