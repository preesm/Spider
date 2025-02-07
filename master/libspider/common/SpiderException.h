/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2013 - 2019) :
 *
 * Antoine Morvan <antoine.morvan@insa-rennes.fr> (2018 - 2019)
 * Clément Guy <clement.guy@insa-rennes.fr> (2014)
 * Florian Arrestier <florian.arrestier@insa-rennes.fr> (2018 - 2019)
 * Hugo Miomandre <hugo.miomandre@insa-rennes.fr> (2017)
 * Julien Heulot <julien.heulot@insa-rennes.fr> (2013 - 2018)
 * Karol Desnos <karol.desnos@insa-rennes.fr> (2017)
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
#ifndef SPIDER_SPIDEREXCEPTION_H
#define SPIDER_SPIDEREXCEPTION_H

#include <stdexcept>
#include <cstdarg>
#include <cstring>



// Size of 50 minimum is required for the error message associated
#define SPIDER_EXCEPTION_BUFFER_SIZE 300

#define throwHelper(msg, ...)\
    throw SpiderException("SpiderException: %s::%s: " msg, __FILENAME__, __func__, __VA_ARGS__)
#define throwSpiderException(...) throwHelper(__VA_ARGS__, '\0')

class SpiderException : public std::exception {
public:
    explicit SpiderException(const char *msg, ...) : exceptionMessage_{} {
        va_list args;
        va_start(args, msg);
#ifdef _WIN32
        int n = _vsnprintf(exceptionMessage_, SPIDER_EXCEPTION_BUFFER_SIZE, msg, args);
#else
        int n = vsnprintf(exceptionMessage_, SPIDER_EXCEPTION_BUFFER_SIZE, msg, args);
#endif
        if (n > SPIDER_EXCEPTION_BUFFER_SIZE) {
            fprintf(stderr, "SpiderException: ERROR: exception message too big.\n");
            fprintf(stderr, "Partially recovered exception: ");
            fflush(stderr);
        }
    }

    const char *what() const noexcept override {
        return exceptionMessage_;
    }

private:
    char exceptionMessage_[SPIDER_EXCEPTION_BUFFER_SIZE];
};


#endif //SPIDER_SPIDEREXCEPTION_H
