/****************************************************************************
 * Copyright or © or Copr. IETR/INSA (2013): Julien Heulot, Yaset Oliva,    *
 * Maxime Pelcat, Jean-François Nezan, Jean-Christophe Prevotet             *
 *                                                                          *
 * [jheulot,yoliva,mpelcat,jnezan,jprevote]@insa-rennes.fr                  *
 *                                                                          *
 * This software is a computer program whose purpose is to execute          *
 * parallel applications.                                                   *
 *                                                                          *
 * This software is governed by the CeCILL-C license under French law and   *
 * abiding by the rules of distribution of free software.  You can  use,    *
 * modify and/ or redistribute the software under the terms of the CeCILL-C *
 * license as circulated by CEA, CNRS and INRIA at the following URL        *
 * "http://www.cecill.info".                                                *
 *                                                                          *
 * As a counterpart to the access to the source code and  rights to copy,   *
 * modify and redistribute granted by the license, users are provided only  *
 * with a limited warranty  and the software's author,  the holder of the   *
 * economic rights,  and the successive licensors  have only  limited       *
 * liability.                                                               *
 *                                                                          *
 * In this respect, the user's attention is drawn to the risks associated   *
 * with loading,  using,  modifying and/or developing or reproducing the    *
 * software by the user in light of its specific status of free software,   *
 * that may mean  that it is complicated to manipulate,  and  that  also    *
 * therefore means  that it is reserved for developers  and  experienced    *
 * professionals having in-depth computer knowledge. Users are therefore    *
 * encouraged to load and test the software's suitability as regards their  *
 * requirements in conditions enabling the security of their systems and/or *
 * data to be ensured and,  more generally, to use and operate it in the    *
 * same conditions as regards security.                                     *
 *                                                                          *
 * The fact that you are presently reading this means that you have had     *
 * knowledge of the CeCILL-C license and that you accept its terms.         *
 ****************************************************************************/

#ifndef SPIDER_H
#define SPIDER_H

class Archi;
class PiSDFVertex;
class PiSDFParam;
class PiSDFGraph;
class PiSDFEdge;
class SRDAGGraph;
class MemAlloc;
class Scheduler;


typedef enum PiSDFSubType {
	PISDF_SUBTYPE_NORMAL,
	PISDF_SUBTYPE_BROADCAST,
	PISDF_SUBTYPE_FORK,
	PISDF_SUBTYPE_JOIN,
	PISDF_SUBTYPE_END,
	PISDF_SUBTYPE_INPUT_IF,
	PISDF_SUBTYPE_OUTPUT_IF
} PiSDFSubType;

#define MAX_STATS_VERTICES 1000
#define MAX_STATS_PE_TYPES 3

typedef unsigned long 	Time;
typedef long 			Param;

typedef void (*lrtFct)(
		void* inputFIFOs[],
		void* outputFIFOs[],
		Param inParams[],
		Param outParams[]);

typedef enum{
	MEMALLOC_DUMMY,
	MEMALLOC_SPECIAL_ACTOR
}MemAllocType;

typedef enum{
	SCHEDULER_LIST
}SchedulerType;

typedef enum{
	STACK_STATIC,
	STACK_DYNAMIC
}StackType;

typedef struct{
	StackType type;
	const char* name;

	void* start;
	int size;
} StackConfig;

typedef struct{
	MemAllocType memAllocType;
	void* memAllocStart;
	int memAllocSize;

	SchedulerType schedulerType;

	StackConfig archiStack;
	StackConfig pisdfStack;
	StackConfig srdagStack;
	StackConfig transfoStack;
	StackConfig lrtStack;

	bool useGraphOptim;
	bool useActorPrecedence;
} SpiderConfig;

typedef struct{
	Time globalEndTime;

	Time execTime;
	Time schedTime;

	Time mappingTime;
	Time optimTime;
	Time graphTime;

	Time actorTimes[MAX_STATS_VERTICES][MAX_STATS_PE_TYPES];
	Time actorFisrt[MAX_STATS_VERTICES];
	Time actorLast[MAX_STATS_VERTICES];
	Time actorIterations[MAX_STATS_VERTICES][MAX_STATS_PE_TYPES];

	PiSDFVertex* actors[MAX_STATS_VERTICES];
	int nPiSDFActor;

	int nSRDAGActor;
	int nSRDAGEdge;
	int nExecSRDAGActor;

	Time forkTime;
	Time joinTime;
	Time rbTime;
	Time brTime;

	int memoryUsed;

	Time latencies;
}ExecutionStat;

/* TODO Namespace */
namespace Spider{
	void init(SpiderConfig cfg);
	void clean();

	void setMemAllocType(MemAllocType type, int start, int size);
	void setSchedulerType(SchedulerType type);

	void setGraphOptim(bool useGraphOptim);
	void setActorPrecedence(bool useActorPrecedence);
	void setSpecialActorPrecedence(bool useSpecialActorPrecedence);

	void setArchi(Archi* archi);
	void setGraph(PiSDFGraph* pisdf);

	void iterate();
	void idle();

	void printGantt(
			const char* ganttPath,
			const char* latexPath,
			ExecutionStat* stat);

	void printSRDAG(const char* srdagPath);

	void printActorsStat(ExecutionStat* stat);

	PiSDFGraph* getGraph();
	Archi* getArchi();

	/* PiSDF Graph Generation */
	PiSDFGraph* createGraph(
			int nEdges,
			int nParams,
			int nInIfs,
			int nOutIfs,
			int nConfigs,
			int nBodies);

	PiSDFVertex* addBodyVertex(
				PiSDFGraph* graph,
				const char* vertexName, int fctId,
				int nInEdge, int nOutEdge,
				int nInParam);

	PiSDFVertex* addHierVertex(
			PiSDFGraph* graph,
			const char* vertexName,
			PiSDFGraph* subgraph,
			int nInEdge, int nOutEdge,
			int nInParam);

	PiSDFVertex* addSpecialVertex(
			PiSDFGraph* graph,
			PiSDFSubType subType,
			int nInEdge, int nOutEdge,
			int nInParam);

	PiSDFVertex* addConfigVertex(
			PiSDFGraph* graph,
			const char* vertexName, int fctId,
			PiSDFSubType subType,
			int nInEdge, int nOutEdge,
			int nInParam, int nOutParam);

	PiSDFVertex* addInputIf(
			PiSDFGraph* graph,
			const char* name,
			int nInParam);

	PiSDFVertex* addOutputIf(
			PiSDFGraph* graph,
			const char* name,
			int nInParam);

	PiSDFParam* addStaticParam(
			PiSDFGraph* graph,
			const char* name,
			const char* expr);

	PiSDFParam* addStaticParam(
			PiSDFGraph* graph,
			const char* name,
			int value);

	PiSDFParam* addHeritedParam(
			PiSDFGraph* graph,
			const char* name,
			int parentId);

	PiSDFParam* addDynamicParam(
			PiSDFGraph* graph,
			const char* name);

	PiSDFParam* addDependentParam(
			PiSDFGraph* graph,
			const char* name,
			const char* expr);

	PiSDFEdge* connect(
			PiSDFGraph* graph,
			PiSDFVertex* source, int sourcePortId, const char* production,
			PiSDFVertex* sink, int sinkPortId, const char* consumption,
			const char* delay, PiSDFVertex* setter=0, PiSDFVertex* getter=0);

	void addInParam(PiSDFVertex* vertex, int ix, PiSDFParam* param);
	void addOutParam(PiSDFVertex* vertex, int ix, PiSDFParam* param);

	void setTimingOnType(PiSDFVertex* vertex, int peType, const char* timing);
	void isExecutableOnAllPE(PiSDFVertex* vertex);
	void isExecutableOnPE(PiSDFVertex* vertex, int pe);

	void cleanPiSDF();
};

#endif//SPIDER_H