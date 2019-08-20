/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2013 - 2019) :
 *
 * Antoine Morvan <antoine.morvan@insa-rennes.fr> (2018 - 2019)
 * Clément Guy <clement.guy@insa-rennes.fr> (2014)
 * Florian Arrestier <florian.arrestier@insa-rennes.fr> (2018 - 2019)
 * Hugo Miomandre <hugo.miomandre@insa-rennes.fr> (2017)
 * Julien Heulot <julien.heulot@insa-rennes.fr> (2013 - 2016)
 * Yaset Oliva <yaset.oliva@insa-rennes.fr> (2013)
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
#include "Archi.h"
#include "ArchiPE.h"


/* === Init of globalID static member === */

std::uint32_t PE::globalID = 0;

/* === Default routines === */

std::uint64_t defaultZeroCommunicationCost(std::uint64_t) {
    return 0;
}

PE::PE(std::uint32_t hwType,
       std::uint32_t hwID,
       std::uint32_t virtID,
       std::string name,
       SpiderPEType spiderPEType,
       SpiderHWType spiderHWType) : hwType_{hwType},
                                    hwID_{hwID},
                                    virtID_{virtID},
                                    name_{std::move(name)},
                                    spiderPEType_{spiderPEType},
                                    spiderHWType_{spiderHWType} {
    spiderID_ = PE::globalID++;

    /* === Initializing read / send CommunicationCostRoutine to default values === */

    auto nPE = Spider::getArchi()->getNPE();
    readCostRoutineArray_ = CREATE_MUL_NA(ARCHI_STACK, nPE, CommunicationCostRoutine);
    sendCostRoutineArray_ = CREATE_MUL_NA(ARCHI_STACK, nPE, CommunicationCostRoutine);
    if (!readCostRoutineArray_ || !sendCostRoutineArray_) {
        throwSpiderException("Failed to allocate cost routine arrays");
    }
    for (std::uint32_t i = 0; i < nPE; ++i) {
        readCostRoutineArray_[i] = defaultZeroCommunicationCost;
        sendCostRoutineArray_[i] = defaultZeroCommunicationCost;
    }
}

PE::~PE() {
    StackMonitor::free(ARCHI_STACK, readCostRoutineArray_);
    StackMonitor::free(ARCHI_STACK, sendCostRoutineArray_);
}