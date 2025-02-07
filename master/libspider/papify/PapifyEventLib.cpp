/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2018 - 2019) :
 *
 * Antoine Morvan <antoine.morvan@insa-rennes.fr> (2018 - 2019)
 * Daniel Madroñal <daniel.madronal@upm.es> (2019)
 * Florian Arrestier <florian.arrestier@insa-rennes.fr> (2018 - 2019)
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
#include <papi.h>
#include <string>
#include <SpiderException.h>
#include "PapifyEventLib.h"

PapifyEventLib::~PapifyEventLib() {
    if (configLock_) {
        pthread_mutex_destroy(configLock_);
        delete configLock_;
    }
    PAPI_shutdown();
}

static void PAPIInitMultiplex() {
    //, *hw_info_register32;
    //, *cmpinfo_register32;

    /* Initialize the library */

    /* For now, assume multiplexing on CPU compnent only */
    const PAPI_component_info_t *cmpinfo = PAPI_get_component_info(0);
    if (cmpinfo == nullptr) {
        PapifyEventLib::throwError(__FILE__, __LINE__, "PAPI_get_component_info failed.");
        return;
    }

    const PAPI_hw_info_t *hw_info = PAPI_get_hardware_info();
    if (hw_info == nullptr) {
        PapifyEventLib::throwError(__FILE__, __LINE__, "PAPI_get_hardware_info failed.");
        return;
    }


    std::string cmpInfoName(cmpinfo->name);
    std::string hwInfoModel(hw_info->model_string);
    if (cmpInfoName.find("perfctr.c") && (hwInfoModel == "POWER6")) {
        int retVal = PAPI_set_domain(PAPI_DOM_ALL);
        if (retVal != PAPI_OK) {
            PapifyEventLib::throwError(__FILE__, __LINE__, retVal);
        }
    }
    int retVal = PAPI_multiplex_init();
    if (retVal != PAPI_OK) {
        PapifyEventLib::throwError(__FILE__, __LINE__, retVal);
    }
}

PapifyEventLib::PapifyEventLib() {
    // Init PAPI
    int retVal = PAPI_library_init(PAPI_VER_CURRENT);
    if (retVal != PAPI_VER_CURRENT) {
        throwError(__FILE__, __LINE__, retVal);
    }

    // Place for initialization in case one makes use of threads
    retVal = PAPI_thread_init((unsigned long (*)()) (pthread_self));
    if (retVal != PAPI_OK) {
        throwError(__FILE__, __LINE__, retVal);
    }

    // Multiplex initialization
    PAPIInitMultiplex();

    printf("PapifyEventLib: init done.\n");
    // Initialize the zero time variable
    zeroTime_ = PAPI_get_real_usec();
    configLock_ = new pthread_mutex_t;
    if (pthread_mutex_init(configLock_, nullptr)) {
        throwError(__FILE__, __LINE__, "mutex init failed.");
    }
}

void PapifyEventLib::throwError(const char *file, int line, const char *message) {
    fprintf(stderr, "File: %s\n", file);
    fprintf(stderr, "Line: %d\n", line);
    throwSpiderException("%s", message);
}

void PapifyEventLib::throwError(const char *file, int line, int papiErrorCode) {
    const char *papiError = PAPI_strerror(papiErrorCode);
    if (papiError) {
        std::string errorMessage = std::string("PAPI Error: ") + std::string(papiError);
        throwError(file, line, errorMessage.c_str());
    }
    throwError(file, line, "an error occured");
}


int PapifyEventLib::PAPIEventSetInit(int numberOfEvents,
                                     std::vector<const char *> &moniteredEventSet,
                                     int eventSetID,
                                     const char *PEType,
                                     const char *PEId,
                                     std::vector<int> &PAPIEventCodeSet) {
    // 1. Retrieve the PAPI event codes
    for (int i = 0; i < numberOfEvents; ++i) {
        const char *eventName = moniteredEventSet[i];
        int retVal = PAPI_event_name_to_code(eventName, &PAPIEventCodeSet[i]);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
        }
    }

    // 2. Create the unified event list
    int eventCodeSetMaxSize = PAPI_get_opt(PAPI_MAX_MPX_CTRS, nullptr);
    if (eventCodeSetMaxSize < numberOfEvents) {
        throwError(__FILE__, __LINE__, "eventCodeSetMaxSize < eventCodeSetSize, too many performance events defined!");
        return -1;
    }
    return registerNewThread(numberOfEvents, PEType, PEId, eventSetID, PAPIEventCodeSet);
}

int PapifyEventLib::registerNewThread(int numberOfEvents,
                                      const char *PEType,
                                      const char *PEId,
                                      int eventSetID,
                                      std::vector<int> &PAPIEventCodeSet) {
    // 1. Register thread
    int retVal = PAPI_register_thread();
    if (retVal != PAPI_OK) {
        throwError(__FILE__, __LINE__, retVal);
        return -1;
    }
    // 2. Create the event set
    int PAPIEventSetID = PAPI_NULL;
    retVal = PAPI_create_eventset(&PAPIEventSetID);
    if (retVal != PAPI_OK) {
        throwError(__FILE__, __LINE__, retVal);
        return -1;
    }
    // 3. Assign the event set to the PE component
    retVal = PAPI_assign_eventset_component(PAPIEventSetID, PAPI_get_component_index(PEType));
    if (retVal == PAPI_ENOCMP) {
        retVal = PAPI_assign_eventset_component(PAPIEventSetID, 0);
    }

    // 4. Checking if multiplexing is necessary or not
    int maxNumberHwCounters = PAPI_get_opt(PAPI_MAX_HWCTRS, nullptr);

    if (maxNumberHwCounters < numberOfEvents) {
        PAPI_option_t opt;
        opt.multiplex.eventset = PAPIEventSetID;
        opt.multiplex.ns = 100;
        opt.itimer.ns = 100;
        retVal = PAPI_set_opt(PAPI_DEF_ITIMER_NS, &opt);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
            return -1;
        }
        retVal = PAPI_set_opt(PAPI_MULTIPLEX, &opt);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
            return -1;
        }
        fprintf(stdout, "Monitoring %d events --> Multiplexing (Max HW counters = %d).\n", numberOfEvents,
                maxNumberHwCounters);
    } else {
        fprintf(stdout, "Monitoring %d events --> No multiplexing needed.\n", numberOfEvents);
    }

    // 5. Register the events
    for (int i = 0; i < numberOfEvents; ++i) {
        PAPI_event_info_t info;
        retVal = PAPI_get_event_info(PAPIEventCodeSet[i], &info);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
            return -1;
        }
        retVal = PAPI_add_event(PAPIEventSetID, info.event_code);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
            return -1;
        }
    }

    // The hash table between the user event set ID and the PAPI event set ID
    PEEventSets_[PEId][eventSetID] = PAPIEventSetID;
    return PAPIEventSetID;
}


void PapifyEventLib::getPAPIEventCodeSet(std::vector<const char *> &moniteredEventSet,
                                         std::vector<int> &PAPIEventCodeSet) {
    unsigned long numberOfEvents = moniteredEventSet.size();
    for (unsigned long i = 0; i < numberOfEvents; ++i) {
        const char *eventName = moniteredEventSet[i];
        int retVal = PAPI_event_name_to_code(eventName, &PAPIEventCodeSet[i]);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
        }
    }
}
