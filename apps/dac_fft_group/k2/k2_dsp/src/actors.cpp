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

#include <platform.h>

#include "actors.h"
#include "data_sp.h"

extern "C"{
#include "ti/dsplib/src/DSPF_sp_fftSPxSP/DSPF_sp_fftSPxSP.h"
#include "edma.h"
}

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <c6x.h>

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#define VERBOSE 0

unsigned char brev[64] = {
    0x0, 0x20, 0x10, 0x30, 0x8, 0x28, 0x18, 0x38,
    0x4, 0x24, 0x14, 0x34, 0xc, 0x2c, 0x1c, 0x3c,
    0x2, 0x22, 0x12, 0x32, 0xa, 0x2a, 0x1a, 0x3a,
    0x6, 0x26, 0x16, 0x36, 0xe, 0x2e, 0x1e, 0x3e,
    0x1, 0x21, 0x11, 0x31, 0x9, 0x29, 0x19, 0x39,
    0x5, 0x25, 0x15, 0x35, 0xd, 0x2d, 0x1d, 0x3d,
    0x3, 0x23, 0x13, 0x33, 0xb, 0x2b, 0x1b, 0x3b,
    0x7, 0x27, 0x17, 0x37, 0xf, 0x2f, 0x1f, 0x3f
};

//#pragma DATA_SECTION(".twiddles")
//static float twi64k_r[64*1024];
//#pragma DATA_SECTION(".twiddles")
//static float twi64k_i[64*1024];

#pragma DATA_SECTION(".twiddles")
static float twi64k[2*64*1024];

//static float gen_twi32k[2*32*1024];
//static float gen_twi16k[2*16*1024];
//static float gen_twi8k [2*8*1024];
//static float gen_twi4k [2*4*1024];
//static float gen_twi2k [2*2*1024];
//static float gen_twi1k [2*1024];

#pragma DATA_SECTION(".twiddles")
static float gen_twi256[2*256];

void tw_gen (float *w, int n);

void initActors(){
//	for(int i=0; i<64*1024; i++){
//		twi64k_r[i] = cos(-2*M_PI*i/(64*1024));
//		twi64k_i[i] = sin(-2*M_PI*i/(64*1024));
//	}

	for(int i=0; i<64*1024; i++){
		twi64k[2*i  ] = -sin(-2*M_PI*i/(64*1024));
		twi64k[2*i+1] =  cos(-2*M_PI*i/(64*1024));
	}

//	tw_gen(gen_twi32k, 32*1024);
//	tw_gen(gen_twi16k, 16*1024);
//	tw_gen(gen_twi8k,   8*1024);
//	tw_gen(gen_twi4k,   4*1024);
//	tw_gen(gen_twi2k,   2*1024);
//	tw_gen(gen_twi1k,     1024);
	tw_gen(gen_twi256,     256);

	edma_init();
}

/* Function for generating Specialized sequence of twiddle factors */
void tw_gen (float *w, int n)
{
    int i, j, k;
    const double PI = 3.141592654;

    for (j = 1, k = 0; j <= n >> 2; j = j << 2)
    {
        for (i = 0; i < n >> 2; i += j)
        {
#ifdef _LITTLE_ENDIAN
            w[k]     = (float) sin (2 * PI * i / n);
            w[k + 1] = (float) cos (2 * PI * i / n);
            w[k + 2] = (float) sin (4 * PI * i / n);
            w[k + 3] = (float) cos (4 * PI * i / n);
            w[k + 4] = (float) sin (6 * PI * i / n);
            w[k + 5] = (float) cos (6 * PI * i / n);
#else
            w[k]     = (float)  cos (2 * PI * i / n);
            w[k + 1] = (float) -sin (2 * PI * i / n);
            w[k + 2] = (float)  cos (4 * PI * i / n);
            w[k + 3] = (float) -sin (4 * PI * i / n);
            w[k + 4] = (float)  cos (6 * PI * i / n);
            w[k + 5] = (float) -sin (6 * PI * i / n);
#endif
            k += 6;
        }
    }
}

float snr(float* sig, float* ref, int n){
	float diff[2*N_DATA];
	int i;
	for(i=0; i<n; i++){
		diff[2*i]   = sig[2*i]   - ref[2*i];
		diff[2*i+1] = sig[2*i+1] - ref[2*i+1];
	}

	float rms_sig = 0;
	float rms_diff = 0;
	for(int i=0; i<n; i++){
		rms_sig  += sig[2*i]*sig[2*i] + sig[2*i+1]*sig[2*i+1];
		rms_diff += diff[2*i]*diff[2*i] + diff[2*i+1]*diff[2*i+1];
	}
	rms_sig  = sqrt(rms_sig/n);
	rms_diff = sqrt(rms_diff/n);

//	printf("RMS Signal : %f\n", rms(sig, n));
//	printf("RMS Noise : %f\n", rms(diff, n));

	return 20*log(rms_sig/rms_diff);
}

void cfg(Param size, Param* Nc, Param* Nr, Param* n1, Param* n2){
#if VERBOSE
	printf("Execute Cfg\n");
#endif
	*Nc = 1<<((int)log2(size)/2);
	*Nr = size/(*Nc);
	*n1 = *Nc/8;
	*n2 = *Nr/8;
}

void src(Param size, float *out){
#if VERBOSE
	printf("Execute Src\n");
#endif
	if(N_DATA != size)
		printf("Bad size, bad SNR expected\n");

//    memcpy(out, data_in, size*2*sizeof(float));
}

void snk(Param size, float *in){
#if VERBOSE
	printf("Execute Snk\n");
#endif

    // Compute SNR
//    float snrVal = snr(in, data_out, size);
//    printf("SNR %f dB\n", snrVal);
}

void fft(Param size, Param n, float* in, float* out){

#if VERBOSE
	printf("Execute fft\n");
#endif

	int rad;
	int j = 0;
	for (int i = 0; i <= 31; i++)
		if ((size & (1 << i)) == 0)
			j++;
		else
			break;

	if (j % 2 == 0)
		rad = 4;
	else
		rad = 2;

	float* w;
	switch(size){
//	case 32*1024:
//		w = gen_twi32k;
//		break;
//	case 16*1024:
//		w = gen_twi16k;
//		break;
//	case  8*1024:
//		w = gen_twi8k;
//		break;
//	case  4*1024:
//		w = gen_twi4k;
//		break;
//	case  2*1024:
//		w = gen_twi2k;
//		break;
//	case    1024:
//		w = gen_twi1k;
//		break;
	case    256:
		w = gen_twi256;
		break;
	default:
		printf("Error no twiddles computed for %d\n", size);
		return;
	}

//	float w[2*size];
//	tw_gen(w, size);

	for(int i=0; i<n; i++){
	    DSPF_sp_fftSPxSP(size, in, w, out, brev, rad, 0, size);
		in += 2*size;
		out += 2*size;
	}

}

void twiddles(Param size, Param n, int* ix, float* restrict in, float* restrict out){
#if VERBOSE
	printf("Execute twiddles\n");
#endif
	int r = (*ix)*n;

//	_nassert((int) in  % 8 == 0); 	// in  is 64-bit aligned
//	_nassert((int) out % 8 == 0); 	// out is 64-bit aligned
//	_nassert((int) twi64k_r % 8 == 0); 	// out is 64-bit aligned
//	_nassert((int) twi64k_i % 8 == 0); 	// out is 64-bit aligned
//
//	for(int i=0; i<n; i++){
//#pragma MUST_ITERATE(8,,8)
//		for(int c=0; c<size; c++){
//			out[2*c  ] = in[2*c]*twi64k_r[r*c] - in[2*c+1]*twi64k_i[r*c];
//			out[2*c+1] = in[2*c]*twi64k_i[r*c] + in[2*c+1]*twi64k_r[r*c];
//		}
//		in += 2*size;
//		out += 2*size;
//		r++;
//	}

	double const* restrict pd_in  = (double*) in;
	double const* restrict pd_twi = (double*) twi64k;
	double * restrict pd_out = (double*) out;

	/* Specify aligned input/output/twi for optimizations */
	_nassert((int) pd_in  % 8 == 0); 	// input  is 64-bit aligned
	_nassert((int) pd_out % 8 == 0); 	// output is 64-bit aligned
	_nassert((int) pd_twi % 8 == 0); 	// twiddles is 64-bit aligned

	for(int i=0; i<n; i++){
#pragma MUST_ITERATE(8, ,8)
		for(int c=0; c<size; c++){
			/* Get the input data */
			__float2_t  v_in  = _amem8_f2_const(&pd_in[c]);

			/* Get the m/q twiddle factor */
			__float2_t  v_twi  = _amem8_f2_const(&pd_twi[r*c]);

			/* Execute:
			 * 		out = in*twi
			 * 		[even]: real part and [odd]: imag part
			 */
			_amem8_f2(&pd_out[c]) = _complex_mpysp(v_in, v_twi);
		}
		pd_in += size;
		pd_out += size;
		r++;
	}
}

void genIx(Param Nr, Param n, int* ixs){
#if VERBOSE
	printf("Execute genIx\n");
#endif
	for(int i=0; i<Nr/n; i++){
		ixs[i] = i;
	}
}
