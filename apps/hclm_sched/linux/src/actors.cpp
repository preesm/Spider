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

#include "actors.h"
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERBOSE 0
#define NB_TAPS 512
#define TEST 1

#if DSP
extern "C"{
#include <ti/dsplib/src/DSPF_sp_fir_gen/DSPF_sp_fir_gen.h>
}
#endif

void cfg_N(Param NVal, Param MStart, Param MNext, Param NMax, Param *N, char* M){
#if VERBOSE
	printf("Execute Cfg_N %ld\n", NVal);
#endif

	*N = NVal;

	for(int i=0; i<NVal; i++)
		M[i] = MStart+i*MNext;
}

void cfg_M(char* in, Param *M){
#if VERBOSE
	printf("Execute Cfg_M %d\n", *in);
#endif

	// Set parameter's value.
	*M = *in;
}

void src(Param NbS, Param N, float* out){
#if VERBOSE
	printf("Execute Src\n");
#endif

	if(TEST){
		for(int i=0; i<N; i++){
			srand(1000);
			for(int j=0; j<NbS; j++){
				out[j+i*NbS] = 10*((float)rand())/RAND_MAX;
			}
		}
	}
}

void snk(Param NbS, Param N, float* in, char* M){
#ifdef DSP
	const int expectedHash[13]={
		/*0 */ 0x06088D61,	/*1 */ 0x03902DCA,	/*2 */ 0x7BE6A549,
		/*3 */ 0x7D008129,	/*4 */ 0x152FE04C,	/*5 */ 0x61FA4D45,
		/*6 */ 0x762F3E52,	/*7 */ 0x1A0D6EA7,	/*8 */ 0x407294F4,
		/*9 */ 0x28182904,	/*10*/ 0x60F32492,	/*11*/ 0x630A18F6,
		/*12*/ 0x476BCCAB
	};
#else
	const int expectedHash[13]={
		/*0 */ 0x1D8CCC7, 	/*1 */ 0x69D0FCD, 	/*2 */ 0x9CA48CA,
		/*3 */ 0x95CFC62, 	/*4 */ 0x5CAE39A, 	/*5 */ 0x170030E8,
		/*6 */ 0x16C43A1F,	/*7 */ 0x1F40E5E3,	/*8 */ 0x4D50F02B,
		/*9 */ 0x7D6D759, 	/*10*/ 0x49638F8,	/*11*/ 0x7BD1349F,
		/*12*/ 0x712A25F4
	};
#endif

#if VERBOSE
	printf("Execute Snk\n");
#endif

	if(TEST){
		int hash;
		for(int i=0; i<N; i++){
			hash = 0;
			int* data = (int*)in;
			for(int j=0; j<NbS; j++){
				hash = hash ^ data[j+i*NbS];
			}
			if(hash != expectedHash[M[i]]){
				printf("ERROR: Bad Hash result for %d M=%d  : %#X instead of %#X\n", i, M[i], hash, expectedHash[M[i]]);
			}else
				printf("Result %d M=%d OK\n", i, M[i]);
		}
	}
}

void initSw(Param M, char* ixs, char* sels){
#if VERBOSE
	printf("Execute initSwitch\n");
#endif

	sels[0] = 0;
	memset(sels+1, 1, M-1);
}

void Switch(Param NbS, char* sel, float* i0, float* i1, float* out){
#if VERBOSE
	printf("Execute Switch\n");
#endif

	switch(*sel){
	case 0:
		memcpy(out, i0, NbS*sizeof(float));
		break;
	case 1:
		memcpy(out, i1, NbS*sizeof(float));
		break;
	default:
		printf("Bad sel received in Switch\n");
		break;
	}
}

void FIR(Param NbS, char*ix,  float* in, float* out){
	float taps[NB_TAPS];

#if VERBOSE
	printf("Execute FIR\n");
#endif

	for(int i=0; i<NB_TAPS; i++){
		taps[i] = 1.0/NB_TAPS;
	}

//	memcpy(out, in, NbS*sizeof(float));

#if DSP
	float input[NbS+NB_TAPS-1];

	memset(input, 0, (NB_TAPS-1)*sizeof(float));
	memcpy(input+NB_TAPS-1, in, NbS*sizeof(float));

//	printf("Inputs:\n in 0x%x \n out 0x%x\n", in, out);

	DSPF_sp_fir_gen(
			input,
			taps,
			out,
			NB_TAPS,
			NbS
	);
#else

	int last_id = 0;
	float last[NB_TAPS];

	memset(last,0,NB_TAPS*sizeof(float));

	for(int i=0; i<NbS; i++){
		out[i] = 0;
		last[last_id] = in[i];
		for(int j=0; j<NB_TAPS; j++){
			out[i] += taps[j]*last[(last_id+j)%NB_TAPS];
		}
		last_id = (last_id+1)%NB_TAPS;
	}
#endif
}
