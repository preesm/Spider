
/****************************************************************************
 * Copyright or � or Copr. IETR/INSA (2013): Julien Heulot, Yaset Oliva,	*
 * Maxime Pelcat, Jean-Fran�ois Nezan, Jean-Christophe Prevotet				*
 * 																			*
 * [jheulot,yoliva,mpelcat,jnezan,jprevote]@insa-rennes.fr					*
 * 																			*
 * This software is a computer program whose purpose is to execute			*
 * parallel applications.													*
 * 																			*
 * This software is governed by the CeCILL-C license under French law and	*
 * abiding by the rules of distribution of free software.  You can  use, 	*
 * modify and/ or redistribute the software under the terms of the CeCILL-C	*
 * license as circulated by CEA, CNRS and INRIA at the following URL		*
 * "http://www.cecill.info". 												*
 * 																			*
 * As a counterpart to the access to the source code and  rights to copy,	*
 * modify and redistribute granted by the license, users are provided only	*
 * with a limited warranty  and the software's author,  the holder of the	*
 * economic rights,  and the successive licensors  have only  limited		*
 * liability. 																*
 * 																			*
 * In this respect, the user's attention is drawn to the risks associated	*
 * with loading,  using,  modifying and/or developing or reproducing the	*
 * software by the user in light of its specific status of free software,	*
 * that may mean  that it is complicated to manipulate,  and  that  also	*
 * therefore means  that it is reserved for developers  and  experienced	*
 * professionals having in-depth computer knowledge. Users are therefore	*
 * encouraged to load and test the software's suitability as regards their	*
 * requirements in conditions enabling the security of their systems and/or *
 * data to be ensured and,  more generally, to use and operate it in the 	*
 * same conditions as regards security. 									*
 * 																			*
 * The fact that you are presently reading this means that you have had		*
 * knowledge of the CeCILL-C license and that you accept its terms.			*
 ****************************************************************************/

#include "lrt_cfg.h"
#include "string.h"
#include <hwQueues.h>
#include <print.h>
#include <platform.h>
#include "lrt_taskMngr.h"
#include "lrt_debug.h"
#include "lrt_amMngr.h"
#include "lrt_actorMngr.h"


/*
*********************************************************************************************************
*                                         GLOBAL DECLARATIONS
*********************************************************************************************************
*/

/* GLOBAL VARIABLES */
FUNCTION_TYPE functions_tbl[NB_LOCAL_FUNCTIONS]; /* Table of Action Fcts */


static UINT8 OSTaskCtr = 0;                     			// Tasks' counter.
static UINT8 OSTaskIndex = 0;                     			// Tasks' index.

//static UINT8* workingMemory;
static UINT8 workingMemory[WORKING_MEMORY_SIZE];
static UINT8* freeWorkingMemoryPtr;

extern void amTaskStart();


/*
*********************************************************************************************************
*                                     CREATE A LRT's TASK
*
* Description: Creates a Lrt's task.
*
*********************************************************************************************************
*/

void  LrtTaskCreate (){
	// Popping second incoming word, the task Id.
//	UINT8 id = RTQueuePop_UINT32(RTCtrlQueue);
	OS_TCB *new_tcb;
	UINT32 taskFunctId;
	LRTActor* newActor;

	if(OSTaskIndex >= OS_MAX_TASKS){
		zynq_puts("Create Task ");zynq_putdec(OSTaskIndex);zynq_puts("\n");
		exitWithCode(1003);
	}
	new_tcb = &OSTCBTbl[OSTaskIndex];
	newActor = &LRTActorTbl[OSTaskIndex];

	if (new_tcb->OSTCBState == OS_STAT_UNINITIALIZED) { /* Make sure task doesn't already exist at this id  */
		new_tcb->OSTCBState = OS_STAT_READY;/* Reserve the priority to prevent others from doing ...  */

		/* Store task ID */
		new_tcb->OSTCBId = OSTaskIndex++; OSTaskCtr++;

		/* Update current running Task List */
		if(OSTCBCur == (OS_TCB*)0){
			/* If no running Task */
			OSTCBCur = new_tcb;
//			new_tcb->OSTCBNext = new_tcb;
//		}else{
//			new_tcb->OSTCBNext = OSTCBCur;
		}

		// Popping the task function id.
		taskFunctId = RTQueuePop_UINT32(RTCtrlQueue);

		// Popping whether the task is stopped after completion.
//		new_tcb->stop = RTQueuePop_UINT32(RTCtrlQueue);

		// Popping the AM flag.
		new_tcb->isAM = RTQueuePop_UINT32(RTCtrlQueue);

		if(new_tcb->isAM){
			// Popping the actor machine's info.
			new_tcb->am.nbVertices 	= RTQueuePop_UINT32(RTCtrlQueue);
			new_tcb->am.nbConds 	= RTQueuePop_UINT32(RTCtrlQueue);
			new_tcb->am.nbActions	= RTQueuePop_UINT32(RTCtrlQueue);

			// Popping the starting vertex of the AM.
			new_tcb->am.currVertexId = RTQueuePop_UINT32(RTCtrlQueue);
			new_tcb->task_func = amTaskStart; // An AM task's function is predefined.
			new_tcb->stop = FALSE;
			// Creating the AM.
			AMCreate(&(new_tcb->am));
		}
		else
		{
			new_tcb->actor = newActor;
			new_tcb->task_func = functions_tbl[taskFunctId];
			new_tcb->stop = TRUE;
			createActor(new_tcb->actor);
		}

	    zynq_puts("Create Task ID"); zynq_putdec(new_tcb->OSTCBId);
	    zynq_puts(" @");  zynq_putdec(new_tcb->am.currVertexId);
	    zynq_puts("\n");

#if defined ARM || defined DESKTOP
		char s[8] = "tX_X.gv";
		s[1] = new_tcb->OSTCBId + '0';
		s[3] = cpuId + '0';
		dotWriter(new_tcb, s);
#endif
	}else
		exitWithCode(1012);
}


AM_ACTOR_ACTION_STRUCT* OSCurActionQuery(){
	return &OSTCBCur->am.am_actions[OSTCBCur->am.am_vertices[OSTCBCur->am.currVertexId].actionID];
}


/*
*********************************************************************************************************
*                                            DELETE A TASK
*
* Description: This function allows you to delete a task from the list of available tasks...
*
* Arguments  : id    is the identifier of the task to delete.
* 			   curr_vertex_id is will contain the id. of the current vertex of the deleted task.
*
* Returns    : OS_ERR_NONE             if the call is successful
*              OS_ERR_TASK_NOT_EXIST   if the task you want to delete does not exist.
*
*********************************************************************************************************
*/

void  LrtTaskDeleteCur(){
	UINT8	id = OSTCBCur->OSTCBId;
    OS_TCB    *del_tcb = &OSTCBTbl[id];

    if (del_tcb->OSTCBState == OS_STAT_READY) { /* Make sure task doesn't already exist at this id  */
    	del_tcb->OSTCBState = OS_STAT_UNINITIALIZED;/* Reserve the priority to prevent others from doing ...  */

    	/* Update current running Task List */
//		if(del_tcb->OSTCBNext == del_tcb){
//			OSTCBCur = (OS_TCB*)0;
//			lrt_running = FALSE;
//		}else{
//			OSTCBCur = del_tcb->OSTCBNext;
//		}

    	if (OSTCBCur->OSTCBId < OSTaskIndex)
    		OSTCBCur = &OSTCBTbl[id + 1];
    	else{
			OSTCBCur = (OS_TCB*)0;
			lrt_running = FALSE;
    	}
		/* Decrement the tasks counter */
		OSTaskCtr--;
    }else
    	exitWithCode(1013);
}

void  OSTaskDel (){
	UINT8 taskId   = RTQueuePop_UINT32(RTCtrlQueue);
	UINT8 vectorId = RTQueuePop_UINT32(RTCtrlQueue);
    OS_TCB       *ptcb = &OSTCBTbl[taskId];
    ptcb->stop = TRUE;
    ptcb->am.stopVertexId = vectorId;
    zynq_puts("Stop Task ID"); zynq_putdec(taskId);
    zynq_puts(" at Vector ID");  zynq_putdec(vectorId);
    zynq_puts("\n");
}


void OSWorkingMemoryInit(){
	freeWorkingMemoryPtr = workingMemory;
}

void* OSAllocWorkingMemory(int size){
	void* mem;
	if(freeWorkingMemoryPtr-workingMemory+size > WORKING_MEMORY_SIZE){
		zynq_puts("Asked ");zynq_putdec(size);zynq_puts(" bytes, but ");zynq_putdec(WORKING_MEMORY_SIZE-(int)freeWorkingMemoryPtr+(int)workingMemory);zynq_puts(" bytes available\n");
		exitWithCode(1015);
	}
	mem = freeWorkingMemoryPtr;
	freeWorkingMemoryPtr += size;
	return mem;
}

void OSFreeWorkingMemory(){
	freeWorkingMemoryPtr = workingMemory;
}