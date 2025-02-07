/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2014 - 2019) :
 *
 * Antoine Morvan <antoine.morvan@insa-rennes.fr> (2018 - 2019)
 * Florian Arrestier <florian.arrestier@insa-rennes.fr> (2018 - 2019)
 * Hugo Miomandre <hugo.miomandre@insa-rennes.fr> (2017)
 * Julien Heulot <julien.heulot@insa-rennes.fr> (2014 - 2018)
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
#include <cinttypes>
#include <graphs/SRDAG/SRDAGGraph.h>
#include <SpiderCommunicator.h>
#include <launcher/Launcher.h>
#include <Logger.h>
#include "Launcher.h"


Launcher Launcher::instance_;

Launcher::Launcher() {
    curNParam_ = 0;
    nLaunched_ = 0;
}

Launcher *Launcher::get() {
    return &instance_;
}

void Launcher::sendRepeatJobQueue(bool enable) {
    NotificationMessage message(LRT_NOTIFICATION,
                                enable ? LRT_REPEAT_ITERATION_EN : LRT_REPEAT_ITERATION_DIS,
                                Platform::get()->getLrtIx());
    for (std::uint32_t i = 0; i < Spider::getArchi()->getNActivatedPE(); ++i) {
        Platform::get()->getSpiderCommunicator()->push_notification(i, &message);
    }
}

void Launcher::send_ResetLrtMsg(int) {
//    auto msg = (Message *) Platform::get()->getSpiderCommunicator()->ctrl_start_send(lrtIx, sizeof(Message));
//    msg->globalID_ = MSG_RESET_LRT;
//    Platform::get()->getSpiderCommunicator()->ctrl_end_send(lrtIx, sizeof(Message));
}

void Launcher::send_ClearTimeMsg(int) {
//    auto msg = (ClearTimeMessage *) Platform::get()->getSpiderCommunicator()->ctrl_start_send(lrtIx,
//                                                                                              sizeof(ClearTimeMessage));
//    msg->globalID_ = MSG_CLEAR_TIME;
//    Platform::get()->getSpiderCommunicator()->ctrl_end_send(lrtIx, sizeof(ClearTimeMessage));
}


void Launcher::sendJob(PiSDFScheduleJob *job) {
    /** 1. Push the job **/
    auto *spiderCommunicator = Platform::get()->getSpiderCommunicator();
    auto instance = job->getNumberOfLaunchedInstances();
    auto *job2Send = job->createJobMessage(instance);
    auto jobID = spiderCommunicator->push_job_message(&job2Send);

    /** 2. Send notification **/
    NotificationMessage notificationMessage(JOB_NOTIFICATION, JOB_ADD, Platform::get()->getLrtIx(), jobID);
    spiderCommunicator->push_notification(job->getMappedPE(instance), &notificationMessage);

    /** 3 Update instance number **/
    job->launchNextInstance();

    /** 4. Update number of param to resolve **/
    curNParam_ += job2Send->nParamOUT_;
    nLaunched_++;
}

void Launcher::sendJob(SRDAGScheduleJob *job) {
    /** 1. Push the job **/
    auto *spiderCommunicator = Platform::get()->getSpiderCommunicator();
    auto *job2Send = job->createJobMessage();
    auto jobID = spiderCommunicator->push_job_message(&job2Send);

    /** 2. Send notification **/
    NotificationMessage notificationMessage(JOB_NOTIFICATION, JOB_ADD, Platform::get()->getLrtIx(), jobID);
    spiderCommunicator->push_notification(job->getMappedPE(), &notificationMessage);

    /** 3 Update instance number **/
    job->launchNextInstance();

    /** 4. Update number of param to resolve **/
    curNParam_ += job2Send->nParamOUT_;
    nLaunched_++;
}

void Launcher::resolveParams(Archi */*archi*/, SRDAGGraph *topDag) {
    while (curNParam_) {
        NotificationMessage message;
        if (Platform::get()->getSpiderCommunicator()->pop_notification(Platform::get()->getNLrt(), &message, true)) {
            if (message.getType() == JOB_NOTIFICATION && message.getSubType() == JOB_SENT_PARAM) {
                ParameterMessage *parameterMessage;
                Platform::get()->getSpiderCommunicator()->pop_parameter_message(&parameterMessage, message.getIndex());
                SRDAGVertex *vertex = topDag->getVertexFromIx(parameterMessage->getVertexID());
                auto *referenceVertex = vertex->getReference();
                if (vertex->getNOutParam() != parameterMessage->getNParam()) {
                    throwSpiderException("Expected %d parameters -- got %d", vertex->getNOutParam(),
                                         parameterMessage->getParams());
                }
                auto *receivedParams = parameterMessage->getParams();
                for (int i = 0; i < vertex->getNOutParam(); ++i) {
                    auto *param = vertex->getOutParam(i);
                    (*param) = receivedParams[i];
                    auto *referenceParameter = (PiSDFParam *) referenceVertex->getOutParam(i);
                    referenceParameter->setValue((*param));
                    if (Spider::getVerbose()) {
                        auto *parameterName = vertex->getReference()->getOutParam(i)->getName();
                        Logger::print(LOG_GENERAL, LOG_INFO, "Received Parameter: %s -- Value: %" PRId64"\n",
                                      parameterName,
                                      receivedParams[i]);
                    }
                }
                curNParam_ -= vertex->getNOutParam();
                parameterMessage->~ParameterMessage();
                StackMonitor::free(ARCHI_STACK, parameterMessage);
            } else {
                /** Push back the message in the queue, it will be treated later **/
                Platform::get()->getSpiderCommunicator()->push_notification(Platform::get()->getNLrt(), &message);
            }
        }
    }
}

void Launcher::sendTraceSpider(TraceSpiderType type, Time start, Time end) {
    auto lrtID = Platform::get()->getLrtIx();
    // Push message
    auto *traceMessage = CREATE(ARCHI_STACK, TraceMessage)(-1, type, lrtID, start, end);
    auto *spiderCommunicator = Platform::get()->getSpiderCommunicator();
    auto index = spiderCommunicator->push_trace_message(&traceMessage);

    // Push notification
    auto notificationMessage = NotificationMessage(TRACE_NOTIFICATION,
                                                   TRACE_SPIDER,
                                                   lrtID,
                                                   index);
    spiderCommunicator->push_notification(Platform::get()->getNLrt(), &notificationMessage);
    nLaunched_++;
}

void Launcher::sendEnableTrace(int lrtID) {
    auto *spiderCommunicator = Platform::get()->getSpiderCommunicator();
    auto enableTraceMessage = NotificationMessage(TRACE_NOTIFICATION, TRACE_ENABLE, Platform::get()->getLrtIx());
    if (lrtID < 0) {
        for (int i = 0; i < Platform::get()->getNLrt(); ++i) {
            spiderCommunicator->push_notification(i, &enableTraceMessage);
        }
    } else if (lrtID < Platform::get()->getNLrt()) {
        spiderCommunicator->push_notification(lrtID, &enableTraceMessage);
    } else {
        throwSpiderException("Bad LRT id: %d", lrtID);
    }
}

void Launcher::sendDisableTrace(int lrtID) {
    auto *spiderCommunicator = Platform::get()->getSpiderCommunicator();
    auto enableTraceMessage = NotificationMessage(TRACE_NOTIFICATION, TRACE_DISABLE, Platform::get()->getLrtIx());
    if (lrtID < 0) {
        for (int i = 0; i < Platform::get()->getNLrt(); ++i) {
            spiderCommunicator->push_notification(i, &enableTraceMessage);
        }
    } else if (lrtID < Platform::get()->getNLrt()) {
        spiderCommunicator->push_notification(lrtID, &enableTraceMessage);
    } else {
        throwSpiderException("Bad LRT id: %d", lrtID);
    }
}

void Launcher::sendEndNotification(Schedule *schedule) {
    for (std::uint32_t pe = 0; pe < Spider::getArchi()->getNPE(); ++pe) {
        NotificationMessage message(LRT_NOTIFICATION,
                                    LRT_END_ITERATION,
                                    Platform::get()->getLrtIx(),
                                    schedule->getNJobs(pe) - 1);
        Platform::get()->getSpiderCommunicator()->push_notification(pe, &message);
    }
}

void Launcher::sendBroadCastNotification(bool delayBroadcoast) {
    NotificationMessage broadcast(JOB_NOTIFICATION,
                                  delayBroadcoast ? JOB_DELAY_BROADCAST_JOBSTAMP : JOB_BROADCAST_JOBSTAMP,
                                  Platform::get()->getLrtIx());
    for (std::uint32_t i = 0; i < Spider::getArchi()->getNActivatedPE(); ++i) {
        if (i == Platform::get()->getLrtIx()) {
            continue;
        }
        Platform::get()->getSpiderCommunicator()->push_notification(i, &broadcast);
    }
}

int Launcher::getNLaunched() {
    return nLaunched_;
}

void Launcher::rstNLaunched() {
    nLaunched_ = 0;
}



