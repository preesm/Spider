/****************************************************************************
 * Copyright or © or Copr. IETR/INSA (2013): Julien Heulot, Yaset Oliva,    *
 * Maxime Pelcat, Jean-François Nezan, Jean-Christophe Prevotet,            *
 * Hugo Miomandre												            *
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

#ifndef PTHREAD_SPIDER_COMMUNICATOR_H
#define PTHREAD_SPIDER_COMMUNICATOR_H

#include <Message.h>
#include <SpiderCommunicator.h>
#include <tools/Stack.h>
#include <sys/types.h>

// semaphore.h includes _ptw32.h that redefines types int64_t and uint64_t on Visual Studio,
// making compilation error with the IDE's own declaration of said types
#include <semaphore.h>
#ifdef _MSC_VER
	#ifdef int64_t
	#undef int64_t
	#endif

	#ifdef uint64_t
	#undef uint64_t
	#endif
#endif

#include <queue>

class PThreadSpiderCommunicator: public SpiderCommunicator{
public:
	PThreadSpiderCommunicator(int msgSizeMax, int nLrt,
							sem_t* semTrace, sem_t* semFifoSpidertoLRT, sem_t* semFifoLRTtoSpider,
							std::queue<unsigned char>* fTraceWr, std::queue<unsigned char>* fTraceRd);
	~PThreadSpiderCommunicator();

	void setLrtCom(int lrtIx, std::queue<unsigned char>* fIn, std::queue<unsigned char>* fOut);

	void* ctrl_start_send(int lrtIx, int size);
	void ctrl_end_send(int lrtIx, int size);

	int ctrl_start_recv(int lrtIx, void** data);
	void ctrl_end_recv(int lrtIx);

	void* trace_start_send(int size);
	void trace_end_send(int size);

	int trace_start_recv(void** data);
	void trace_end_recv();

private:
	std::queue<unsigned char>** fIn_;
	std::queue<unsigned char>** fOut_;
	std::queue<unsigned char>* fTraceRd_;
	std::queue<unsigned char>* fTraceWr_;

	sem_t* semTrace_;
	sem_t* semFifoSpidertoLRT_;
	sem_t* semFifoLRTtoSpider_;

	int msgSizeMax_;

	void* msgBufferRecv_;
	int curMsgSizeRecv_;
	void* msgBufferSend_;
	int curMsgSizeSend_;
};

#endif/*PTHREADS_SPIDER_COMMUNICATOR_H*/