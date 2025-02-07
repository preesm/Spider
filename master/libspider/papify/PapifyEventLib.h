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
#ifndef SPIDER_PAPIFYEVENTLIB_H
#define SPIDER_PAPIFYEVENTLIB_H

#include <pthread.h>
#include <vector>
#include <papi.h>
#include <map>

class PapifyEventLib {
public:
    /**
     * Initializes the PAPI library and the different interfaces needed
     */
    PapifyEventLib();

    /**
     * Destructor
     */
    ~PapifyEventLib();

    /**
     * @brief Throw an error related to PapifyEventLib
     *
     * @param file          File in which the error happened
     * @param line          Line of the error
     * @param message The message
     */
    static void throwError(const char *file, int line, const char *message);

    /**
     * @brief Throw an error related to PapifyEventLib
     *
     * @param file          File in which the error happened
     * @param line          Line of the error
     * @param papiErrorCode The error code
     */
    static void throwError(const char *file, int line, int papiErrorCode);

    /**
     * @brief Init a PAPI event set corresponding to the event passed in parameter.
     * @param numberOfEvents     Number of events in the event set
     * @param moniteredEventSet  The event set names (provided to the method)
     * @param eventSetID         User id of the event set
     * @param PEType             The PE component type
     * @param PAPIEventCodeSet   The event set codes (initialized by the method)
     *
     * @return the id of the event set
     */
    int PAPIEventSetInit(int numberOfEvents,
                         std::vector<const char *> &moniteredEventSet,
                         int eventSetID,
                         const char *PEType,
                         const char *PEId,
                         std::vector<int> &PAPIEventCodeSet);

    /**
     * @brief Retrieve the PAPI code equivalent to the PAPI name code
     * @param moniteredEventSet  The event set names (provided to the method)
     * @param PAPIEventCodeSet   The event set codes (initialized by the method)
     *
     */
    void getPAPIEventCodeSet(std::vector<const char *> &moniteredEventSet,
                             std::vector<int> &PAPIEventCodeSet);

    /**
     *
     * @param PEId
     */
    int registerNewThread(int numberOfEvents,
                          const char *PEType,
                          const char *PEId,
                          int eventSetID,
                          std::vector<int> &PAPIEventCodeSet);

    /**
     * @brief Add an entry in the table of PE event set ID / event set launched
     *
     * @param PEId PE id
     */
    inline void registerProcessingElement(const char *PEId) {
        try {
            PEEventSets_.at(PEId);
        } catch (std::out_of_range &e) {
            PEEventSets_.insert(std::make_pair(PEId, std::vector<int>(50, PAPI_NULL)));
            PEEventSetRunningID_.insert(std::make_pair(PEId, PAPI_NULL));
            PEEventSetRunning_.insert(std::make_pair(PEId, false));
        }
    }

    /**
     * @brief Returned the zero time of the initialization of PAPI
     *
     * @return the zero time value
     */
    inline long long getZeroTime() {
        return zeroTime_;
    }

    /**
     * @brief Check if a given event set has already been launched
     * @param eventSetID User id (not PAPI) of the event set
     *
     * @return true if already launched, false else
     */
    inline bool isEventSetRunning(const char *PEId, int PAPIEventSetID_) {
        return isSomeEventSetRunning(PEId) && PEEventSetRunningID_[PEId] == PAPIEventSetID_;
    }

    /**
     * @brief Check if some event set is running
     *
     * @param PEId  The PE id
     * @param index Index to be filled with the eventSet ID currently running
     * @return true if an event set is running, false else
     */
    inline bool isSomeEventSetRunning(const char *PEId) {
        return PEEventSetRunning_[PEId];
    }

    inline void stopEventSetRunning(const char *PEId) {
        int retVal = PAPI_stop(PEEventSetRunningID_[PEId], nullptr);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
        }
        PEEventSetRunning_[PEId] = false;
        PEEventSetRunningID_[PEId] = PAPI_NULL;
    }

    /**
     * @brief Check if a given event set has already been created
     * @param eventSetID User id (not PAPI) of the event set
     *
     * @return true if already exists, false else
     */
    inline bool doesEventSetExists(const char *PEId, int eventSetID) {
        if (eventSetID < 0) {
            throwError(__FILE__, __LINE__, "unvalid event set value");
        }
        return PEEventSets_[PEId][eventSetID] != PAPI_NULL;
    }

    /**
     * @brief Launch the monitoring of the eventSet.
     *        This eventSet will be counting from the beginning and the actual values will
     *        be computed by differences with event_start and event_stop functions.
     *
     * @param PAPIEventSetID The PAPI event set id
     */
    inline void startEventSet(const char *PEId, int PAPIEventSetID) {
        int retVal = PAPI_start(PAPIEventSetID);
        if (retVal != PAPI_OK) {
            throwError(__FILE__, __LINE__, retVal);
        }
        PEEventSetRunning_[PEId] = true;
        PEEventSetRunningID_[PEId] = PAPIEventSetID;
    }

    /**
     * @brief Get the PAPI event set ID from the user event set ID
     *
     * @param eventSetID the user event set ID
     * @return the corresponding PAPI event set id
     */
    inline int getPAPIEventSetID(const char *PEId, int eventSetID) {
        return PEEventSets_[PEId][eventSetID];
    }

    /**
     * @brief Lock the access to the eventlib manager
     */
    inline void configLock() {
        pthread_mutex_lock(configLock_);
    }


    /**
     * @brief Unlock the access to the eventlib manager
     */
    inline void configUnlock() {
        pthread_mutex_unlock(configLock_);
    }

private:
    pthread_mutex_t *configLock_;
    std::map<const char *, std::vector<int> > PEEventSets_;
    std::map<const char *, bool> PEEventSetRunning_;
    std::map<const char *, int> PEEventSetRunningID_;
    long long zeroTime_;
};

#endif //SPIDER_PAPIFYEVENTLIB_H
