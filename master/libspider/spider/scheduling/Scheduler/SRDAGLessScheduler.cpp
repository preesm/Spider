/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Antoine Morvan <antoine.morvan@insa-rennes.fr> (2019)
 * Florian Arrestier <florian.arrestier@insa-rennes.fr> (2019)
 *
 * Spider is a dataflow based runtime used to execute dynamic PiSDF
 * applications. The Preesm tool may be used to design PiSDF applications.
 *
 * This software is governed by the CeCILL  license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */
#include "SRDAGLessScheduler.h"
#include <graphs/PiSDF/PiSDFEdge.h>
#include <graphs/PiSDF/PiSDFVertex.h>
#include <graphs/PiSDF/PiSDFGraph.h>
#include <cmath>
#include <graphTransfo/ComputeBRV.h>
#include <cinttypes>
#include <tools/LinkedList.h>
#include <scheduling/MemAlloc.h>
#include <graphs/VirtualPiSDF/VirtualPiSDF.h>

#define SCHEDULE_SIZE 20000

void SRDAGLessScheduler::initiliazeVertexScheduleIR(PiSDFVertex *const vertex, std::int32_t rv) {
    auto vertexIx = getVertexIx(vertex);
    vertex->createScheduleJob(rv);
//    rhoValueArray_[vertexIx] = 1;
    instanceAvlCountArray_[vertexIx] = rv;
}

void SRDAGLessScheduler::replaceInputIfWithBroadcast(PiSDFGraph *const graph) {
    for (int ix = 0; ix < graph->getNInIf(); ++ix) {
        auto *inputIf = graph->getInputIf(ix);
        auto *edge = inputIf->getOutEdge(0);
        auto edgeSnkIx = edge->getSnkPortIx();
        auto *snkVertex = edge->getSnk();
        auto cons = edge->resolveCons();
        auto prod = edge->resolveProd();
        /** Adding broadcast if needed **/
        auto totalCons = cons * snkVertex->getBRVValue();
        if (prod != totalCons) {
            auto *broadcast = graph->addSpecialVertex(
                    /* SubType */ PISDF_SUBTYPE_BROADCAST,
                    /* InData  */ 1,
                    /* OutData */ 1,
                    /* InParam */ 0);
            broadcast->setBRVValue(1);
            broadcast->createScheduleJob(1);
            broadcast->setId(nVertices_ + 1);
            broadcast->isExecutableOnAllPE();
            //broadcast->setTimingOnType(Spider::getArchi()->getSpiderPeIx(), "50");
            specialActorsAdded_.push_back(broadcast);
            snkVertex->disconnectInEdge(edgeSnkIx);
            edge->disconnectSnk();
            edge->connectSnk(broadcast, 0, std::to_string(prod).c_str());
            broadcast->connectInEdge(0, edge);
            graph->connect(broadcast, 0, std::to_string(totalCons).c_str(),
                           snkVertex, edgeSnkIx, edge->getConsExpr()->toString(),
                           "0", nullptr, nullptr, false);
            nVertices_++;
        }
    }
    for (int i = 0; i < graph->getNBody(); ++i) {
        auto *vertex = graph->getBody(i);
        if (vertex->isHierarchical()) {
            replaceInputIfWithBroadcast(vertex->getSubGraph());
        }
    }
}

static void removeBroadcast(PiSDFVertex *vertex) {
    auto *edgeOut = vertex->getOutEdge(0);
    auto *sink = edgeOut->getSnk();
    auto sinkPortIx = edgeOut->getSnkPortIx();
    auto *snkExpr = edgeOut->getConsExpr()->toString();
    auto *edgeIn = vertex->getInEdge(0);
    /** Disconnect / Reconnect sink on the edge **/
    edgeIn->disconnectSnk();
    edgeIn->connectSnk(sink, sinkPortIx, snkExpr);
    /** Disconnect / Reconnect edge on the vertex **/
    sink->disconnectInEdge(sinkPortIx);
    sink->connectInEdge(sinkPortIx, edgeIn);
    /** Remove the vertex and the edge **/
    vertex->getGraph()->delVertex(vertex);
    vertex->getGraph()->delEdge(edgeOut);
}

static std::int32_t computeTotalNBodies(PiSDFGraph *g) {
    std::int32_t nBodies = g->getNBody();
    for (auto i = 0; i < g->getNBody(); ++i) {
        if (g->getBody(i)->isHierarchical()) {
            nBodies += computeTotalNBodies(g->getBody(i)->getSubGraph());
        }
    }
    return nBodies;
}

void SRDAGLessScheduler::initIR(PiSDFGraph *const graph, std::int32_t coeffRV) {
    for (int ix = 0; ix < graph->getNBody(); ++ix) {
        auto *vertex = graph->getBody(ix);
        auto globalRV = coeffRV * vertex->getBRVValue();
        initiliazeVertexScheduleIR(vertex, globalRV);
        if (vertex->isHierarchical()) {
            initIR(vertex->getSubGraph(), globalRV);
        }
    }
}

SRDAGLessScheduler::SRDAGLessScheduler(PiSDFGraph *graph, PiSDFSchedule *schedule) {
    graph_ = graph;
    schedule_ = schedule;
    nVertices_ = computeTotalNBodies(graph);

    /** 0. Check IF **/
    replaceInputIfWithBroadcast(graph);

    /** 1. Creates the array of ScheduleVertex **/
//    rhoValueArray_ = CREATE_MUL_NA(TRANSFO_STACK, nVertices_, std::int32_t);
    instanceAvlCountArray_ = CREATE_MUL_NA(TRANSFO_STACK, nVertices_, std::int32_t);
    instanceSchCountArray_ = CREATE_MUL_NA(TRANSFO_STACK, nVertices_, std::int32_t);
    memset(instanceSchCountArray_, 0, nVertices_ * sizeof(std::int32_t));

    /** 2. Initialize vertices IR **/
    initIR(graph, 1);
//    /** 4. Compute the Rho values **/
//    computeRhoValues();
}

SRDAGLessScheduler::~SRDAGLessScheduler() {
    for (auto &v : specialActorsAdded_) {
        removeBroadcast(v);
    }
    schedule_ = nullptr;
//    StackMonitor::free(TRANSFO_STACK, rhoValueArray_);
    StackMonitor::free(TRANSFO_STACK, instanceAvlCountArray_);
    StackMonitor::free(TRANSFO_STACK, instanceSchCountArray_);
}

void SRDAGLessScheduler::computeRhoValues() {
    /** Compute current value of rho for every actor **/
    for (int v = 0; v < graph_->getNBody(); ++v) {
        auto *vertex = graph_->getBody(v);
        auto *dependencies = dependenciesArray_[v];
        for (int i = 0; i < vertex->getNInEdge(); ++i) {
            auto *srcVertex = dependencies[i].vertex_;
            if (srcVertex->getGraph() != graph_) {
                /** Case of input if **/
                continue;
            }
            /** Compute raw rho value **/
            auto cons = dependencies[i].cons_;
            auto prod = dependencies[i].prod_;
            auto currentMinExec = static_cast<int32_t >(cons / prod + (cons % prod != 0));
            /** Take maximum between current rho value and raw value**/
//                currentMinExec = std::max(currentMinExec, rhoValueArray_[vertex->getTypeId()]);
//                currentMinExec = std::min(currentMinExec, instanceAvlCountArray_[srcVertex->getTypeId()]);
            /** Set the rho value of previous actor **/
            int index = srcVertex->getTypeId();
            // Case at least RHO execution of every successor
//                if (currentMinExec > rhoValueArray_[index]) {
//                    converged = false;
//                    rhoValueArray_[index] = currentMinExec;
//                }
            // Case at least ONE instance of every successor
            rhoValueArray_[index] = std::max(rhoValueArray_[index], currentMinExec);
//                //  Case at least ONE instance of ONE successor
//                if (currentMinExec > rhoValueArray_[index]) {
//                    rhoValueArray_[index] = std::max(rhoValueArray_[index], currentMinExec);
//                    break;
//                }
        }
    }
}

void SRDAGLessScheduler::printRhoValues() {
    fprintf(stderr, "\nINFO: Rho values:\n");
    for (int i = 0; i < nVertices_; i++) {
        fprintf(stderr, "INFO: >> Vertex: %s -- Rho: %d\n", graph_->getBody(i)->getName(),
                rhoValueArray_[i]);
    }
}

Time SRDAGLessScheduler::computeMinimumStartTime(PiSDFVertex *const vertex) {
    Time minimumStartTime = 0;
    auto *job = vertex->getScheduleJob();
    auto instance = instanceSchCountArray_[getVertexIx(vertex)];
    auto *jobConstrains = job->getScheduleConstrain(instance);
    for (int ix = 0; ix < vertex->getNInEdge(); ++ix) {
        std::int32_t deltaStart = 0;
        std::int32_t deltaEnd = 0;
        auto *vertexInSrc = SRDAGLessIR::getProducer(vertex, ix, instance);
        if (vertexInSrc->getType() == PISDF_TYPE_IF) {
            auto edgeIx = ix;
            SRDAGLessIR::computeDependenciesIxFromInputIF(vertex, &edgeIx, instanceSchCountArray_,
                                                          &vertexInSrc,
                                                          &deltaStart,
                                                          &deltaEnd);
            if (deltaStart < 0) {
                /*!< Case of init */
                continue;
            }
        } else {
            deltaStart = SRDAGLessIR::computeFirstDependencyIx(vertex, ix, instance);
            if (deltaStart < 0) {
                /*!< Case of init */
                continue;
            }
            deltaEnd = SRDAGLessIR::computeLastDependencyIx(vertex, ix, instance);
        }
        auto *vertexInSrcJob = vertexInSrc->getScheduleJob();
        for (int i = deltaStart; i <= deltaEnd; ++i) {
            /** Compute the minimal start time **/
            minimumStartTime = std::max(minimumStartTime, vertexInSrcJob->getMappingEndTime(i));
            /** Compute the minimal dependency we need **/
            auto pe = vertexInSrcJob->getMappedPE(i);
            auto currentValue = jobConstrains[pe].jobId_;
            auto srcInstanceJobID = vertexInSrcJob->getJobID(i);
            if (srcInstanceJobID > currentValue) {
                job->setScheduleConstrain(instance, pe, vertexInSrc, srcInstanceJobID, i);
            }
        }
    }
    return minimumStartTime;
}

void SRDAGLessScheduler::mapVertex(PiSDFVertex *const vertex) {
    auto *job = vertex->getScheduleJob();
    Time minimumStartTime = computeMinimumStartTime(vertex);
    int bestSlave = -1;
    Time bestStartTime = 0;
    Time bestEndTime = UINT64_MAX;
    Time bestWaitTime = 0;
    auto *archi = Spider::getArchi();
    for (std::uint32_t pe = 0; pe < archi->getNPE(); ++pe) {
        /** Skip disabled processing elements **/
        if (!archi->getPEFromSpiderID(pe)->isEnabled()) {
            continue;
        }
        /** Search for best candidate **/
        if (vertex->canExecuteOn(pe)) {
            Time startTime = std::max(schedule_->getReadyTime(pe), minimumStartTime);
            Time waitTime = startTime - schedule_->getReadyTime(pe);
            auto peType = archi->getPEFromSpiderID(pe)->getHardwareType();
            Time execTime = vertex->getTimingOnPEType(peType);
            // TODO: add communication time in the balance
            Time endTime = startTime + execTime;
            if ((endTime < bestEndTime) || (endTime == bestEndTime && waitTime < bestWaitTime)) {
                bestStartTime = startTime;
                bestWaitTime = waitTime;
                bestEndTime = endTime;
                bestSlave = pe;
            }
        }
    }
    if (bestSlave < 0) {
        throwSpiderException("No slave found for vertex [%s].", vertex->getName());
    }
    auto instance = instanceSchCountArray_[getVertexIx(vertex)];
    if (instance > vertex->getBRVValue()) {
        fprintf(stderr, "INFO: instance: %d -- brv: %d\n", instance, vertex->getBRVValue());
    }
    job->setMappedPE(instance, bestSlave);
    job->setMappingStartTime(instance, &bestStartTime);
    job->setMappingEndTime(instance, &bestEndTime);
    schedule_->addJob(job, instance);
}

void SRDAGLessScheduler::map(PiSDFVertex *const vertex, MemAlloc *memAlloc) {
    auto vertexIx = getVertexIx(vertex);
    if (vertex->isHierarchical()) {
        auto *subGraph = vertex->getSubGraph();
        schedule(subGraph, memAlloc);
        auto instance = instanceSchCountArray_[vertexIx];
        Time endTime = 0;
        for (int i = 0; i < subGraph->getNOutIf(); ++i) {
            auto *outputIf = subGraph->getOutputIf(i);
            auto *vertexSrc = outputIf->getInEdge(0)->getSrc();
            endTime = std::max(endTime, vertexSrc->getScheduleJob()->getMappingEndTime(vertexSrc->getBRVValue() - 1));
        }
        vertex->getScheduleJob()->setMappingEndTime(instance, &endTime);
    } else {
        mapVertex(vertex);
    }
    /** Updating values **/
    instanceAvlCountArray_[vertexIx]--;
    instanceSchCountArray_[vertexIx]++;
}

bool SRDAGLessScheduler::isSchedulable(PiSDFVertex *const vertex, std::int32_t /*nInstances*/) {
    auto vertexIx = getVertexIx(vertex);
    auto instance = instanceSchCountArray_[vertexIx];
    bool canRun = true;
    for (int32_t ix = 0; ix < vertex->getNInEdge() && canRun; ++ix) {
        auto *vertexInSrc = SRDAGLessIR::getProducer(vertex, ix, instance);
        std::int32_t deltaIx = 0;
        if (vertexInSrc->getType() == PISDF_TYPE_IF) {
            continue;
        } else {
            deltaIx = SRDAGLessIR::computeLastDependencyIx(vertex, ix, instance);
        }
        canRun &= (instanceSchCountArray_[getVertexIx(vertexInSrc)] % (vertexInSrc->getBRVValue() + 1) > deltaIx);
    }
    return canRun;
}

const PiSDFSchedule *SRDAGLessScheduler::schedule(PiSDFGraph *const graph, MemAlloc *memAlloc) {
    /** Alloc memory **/
//    memAlloc->alloc(graph_);
    /** Initialize list **/
    LinkedList<PiSDFVertex *> list(TRANSFO_STACK, nVertices_);
    for (int ix = 0; ix < graph->getNBody(); ++ix) {
        auto *vertex = graph->getBody(ix);
        list.add(vertex);
    }
    /** Iterate on the list **/
    list.setOnFirst();
    auto *node = list.current();
    while (node) {
        auto *vertex = node->val_;
        auto vertexIx = getVertexIx(vertex);
//        auto numberSchedulable = std::min(rhoValueArray_[vertexIx], instanceAvlCountArray_[vertexIx]);
//        auto instance = instanceSchCountArray_[vertexIx];
//        numberSchedulable *= isSchedulable(vertex, numberSchedulable + instance);
        /*!< Check for all remaining instances if it is schedulable */
        auto numberSchedulable = isSchedulable(vertex, 1);
        for (int i = 0; i < numberSchedulable; ++i) {
            /** Map the vertex **/
            map(vertex, memAlloc);
        }
        if (!instanceAvlCountArray_[vertexIx]) {
            /** Remove node as we finished using it **/
            list.del(node);
            node = list.current();
        } else {
            /** Get on next node **/
            node = list.next();
        }
    }
    return schedule_;
}


/*************************************************/
/**              RELAXED VERSION                **/
/*************************************************/

static inline void buildRelaxedList(PiSDFGraph *graph, LinkedList<PiSDFVertex *> &list) {
    for (int ix = 0; ix < graph->getNBody(); ++ix) {
        auto *vertex = graph->getBody(ix);
        if (vertex->isHierarchical()) {
            buildRelaxedList(vertex->getSubGraph(), list);
        } else {
            list.add(vertex);
        }
    }
}

Time SRDAGLessScheduler::computeMinimumStartTimeRelaxed(PiSDFVertex *vertex) {
    Time minimumStartTime = 0;
    auto vertexGlobInst = instanceSchCountArray_[getVertexIx(vertex)];
    auto topVertexGlobInst = vertexGlobInst;
    for (int ix = 0; ix < vertex->getNInEdge(); ++ix) {
        if (SRDAGLessIR::checkForInitIfInputIF(vertex, ix, vertexGlobInst)) {
            continue;
        }
        auto topEdgeIx = ix;
        auto *topVertex = SRDAGLessIR::getTopParentVertex(vertex, ix, vertexGlobInst, &topVertexGlobInst, &topEdgeIx);
        PiSDFVertex *srcVertex = nullptr;
        std::int32_t firstDep = 0;
        std::int32_t lastDep = 0;
        auto *edge = SRDAGLessIR::computeDependenciesIxRelaxed(topVertex,
                                                               topEdgeIx,
                                                               topVertexGlobInst,
                                                               &srcVertex,
                                                               &firstDep,
                                                               &lastDep);
        if (firstDep < 0) {
            /*!< Case of init */
            continue;
        }
        /** Compute minimumStartTime **/
        if (topVertex != vertex && srcVertex != topVertex->getInEdge(topEdgeIx)->getSrc()) {
            auto srcBRV = srcVertex->getBRVValue();
            auto lowerBound = srcBRV - SRDAGLessIR::fastCeilIntDiv(edge->resolveCons(),
                                                                   edge->resolveProd());
            auto upperBound = srcBRV - 1;
            auto *job = vertex->getScheduleJob();
            auto vertexInstance = instanceSchCountArray_[getVertexIx(vertex)];
            auto *jobConstrains = job->getScheduleConstrain(vertexInstance);
            auto *srcVertexJob = srcVertex->getScheduleJob();
            for (int i = firstDep; i <= lastDep; ++i) {
                /** Compute the minimal start time **/
                minimumStartTime = std::max(minimumStartTime, srcVertexJob->getMappingEndTime(i));
                /** Compute the minimal dependency we need **/
                auto pe = srcVertexJob->getMappedPE(i);
                auto currentValue = jobConstrains[pe].jobId_;
                auto srcInstanceJobID = srcVertexJob->getJobID(i);
                if (srcInstanceJobID > currentValue) {
                    job->setScheduleConstrain(vertexInstance, pe, srcVertex, srcInstanceJobID, i);
                }
                if (i % srcBRV == upperBound) {
                    i += (lowerBound);
                }
            }
        } else {
            auto *job = vertex->getScheduleJob();
            auto vertexInstance = instanceSchCountArray_[getVertexIx(vertex)];
            auto *jobConstrains = job->getScheduleConstrain(vertexInstance);
            auto *srcVertexJob = srcVertex->getScheduleJob();
            for (int i = firstDep; i <= lastDep; ++i) {
                /** Compute the minimal start time **/
                minimumStartTime = std::max(minimumStartTime, srcVertexJob->getMappingEndTime(i));
                /** Compute the minimal dependency we need **/
                auto pe = srcVertexJob->getMappedPE(i);
                auto currentValue = jobConstrains[pe].jobId_;
                auto srcInstanceJobID = srcVertexJob->getJobID(i);
                if (srcInstanceJobID > currentValue) {
                    job->setScheduleConstrain(vertexInstance, pe, srcVertex, srcInstanceJobID, i);
                }
            }
        }
    }
    return minimumStartTime;
}

void SRDAGLessScheduler::mapVertexRelaxed(PiSDFVertex *vertex) {
    auto *job = vertex->getScheduleJob();
    Time minimumStartTime = computeMinimumStartTimeRelaxed(vertex);
    int bestSlave = -1;
    Time bestStartTime = 0;
    Time bestEndTime = UINT64_MAX;
    Time bestWaitTime = 0;
    auto *archi = Spider::getArchi();
    for (std::uint32_t pe = 0; pe < archi->getNPE(); ++pe) {
        /** Skip disabled processing elements **/
        if (!archi->getPEFromSpiderID(pe)->isEnabled()) {
            continue;
        }
        /** Search for best candidate **/
        if (vertex->canExecuteOn(pe)) {
            Time startTime = std::max(schedule_->getReadyTime(pe), minimumStartTime);
            Time waitTime = startTime - schedule_->getReadyTime(pe);
            auto peType = archi->getPEFromSpiderID(pe)->getHardwareType();
            Time execTime = vertex->getTimingOnPEType(peType);
            // TODO: add communication time in the balance
            Time endTime = startTime + execTime;
            if ((endTime < bestEndTime) || (endTime == bestEndTime && waitTime < bestWaitTime)) {
                bestStartTime = startTime;
                bestWaitTime = waitTime;
                bestEndTime = endTime;
                bestSlave = pe;
            }
        }
    }
    if (bestSlave < 0) {
        throwSpiderException("No slave found for vertex [%s].", vertex->getName());
    }
    auto instance = instanceSchCountArray_[getVertexIx(vertex)];
    job->setMappedPE(instance, bestSlave);
    job->setMappingStartTime(instance, &bestStartTime);
    job->setMappingEndTime(instance, &bestEndTime);
    schedule_->addJob(job, instance);

}

bool SRDAGLessScheduler::isSchedulableRelaxed(PiSDFVertex *const vertex) {
    auto vertexIx = getVertexIx(vertex);
    auto vertexGlobInst = instanceSchCountArray_[vertexIx];
    auto topVertexGlobInst = vertexGlobInst;
    bool canRun = true;
    for (int32_t ix = 0; ix < vertex->getNInEdge() && canRun; ++ix) {
        auto topEdgeIx = ix;
        auto *topVertex = SRDAGLessIR::getTopParentVertex(vertex, ix, vertexGlobInst, &topVertexGlobInst, &topEdgeIx);
        PiSDFVertex *srcVertex = nullptr;
        std::int32_t deltaIx = SRDAGLessIR::computeLastDependencyIxRelaxed(topVertex,
                                                                           topEdgeIx,
                                                                           topVertexGlobInst,
                                                                           &srcVertex);
        canRun &= (instanceSchCountArray_[getVertexIx(srcVertex)] > deltaIx);
    }
    return canRun;
}

const PiSDFSchedule *SRDAGLessScheduler::scheduleRelaxed(PiSDFGraph *const graph, MemAlloc */*memAlloc*/) {
    /** Initialize list **/
    LinkedList<PiSDFVertex *> list(TRANSFO_STACK, nVertices_);
    buildRelaxedList(graph, list);
    /** Iterate on the list **/
    list.setOnFirst();
    auto *node = list.current();
    while (node) {
        auto *vertex = node->val_;
        auto vertexIx = getVertexIx(vertex);
        /*!< Check for all remaining instances if it is schedulable */
        auto numberSchedulable = isSchedulableRelaxed(vertex);
        for (int i = 0; i < numberSchedulable; ++i) {
            /** Map the vertex **/
            mapVertexRelaxed(vertex);
            /** Updating values **/
            instanceAvlCountArray_[vertexIx]--;
            instanceSchCountArray_[vertexIx]++;
        }
        if (!instanceAvlCountArray_[vertexIx]) {
            /** Remove node as we finished using it **/
            list.del(node);
            node = list.current();
        } else {
            /** Get on next node **/
            node = list.next();
        }
    }
    return schedule_;
}













