
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

#ifndef OS_CFG_H
#define OS_CFG_H

#define OS_LOWEST_PRIO 				10
#define OS_MAX_TASKS 				10
#define NB_LOCAL_FUNCTIONS 			10
#define OS_DEBUG_EN 				1
#define CONTROL_COMM				0

#define SCHED_POLICY_RR				0
#define SCHED_POLICY_FP				0
#define ACTOR_MACHINE				1

#define OS_MAX_SH_MEM				1
#define OS_MAX_CTRL_Q				2


//#define AM_STATE_MAX_CONDITIONS	2
//#define AM_MAX_NB_EDGES			100

#if defined ARM || defined DESKTOP
#define AM_MAX_NB_VERTICES		300
#define AM_MAX_NB_ACTIONS		60
#define AM_MAX_NB_CONDITIONS	60
#else
#define AM_MAX_NB_VERTICES		200
#define AM_MAX_NB_CONDITIONS	50
#define AM_MAX_NB_ACTIONS		20
#endif

#define AM_MAX_NB_SUCCESSORS	2
#define OS_NB_FIFO		150

#if defined ARM || defined DESKTOP
#define MAX_NB_ARGS		OS_NB_FIFO
#define MAX_NB_FIFO		OS_NB_FIFO
#define WORKING_MEMORY_SIZE 720*400*3
#else
#define MAX_NB_ARGS		50
#define MAX_NB_FIFO		100
#define WORKING_MEMORY_SIZE 720*(400/8)
#endif

#endif