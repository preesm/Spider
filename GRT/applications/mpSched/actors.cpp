
#include "actors.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <grt_definitions.h>

#ifdef DSP
extern "C"{
#include <ti/dsplib/src/DSPF_sp_fir_gen/DSPF_sp_fir_gen.h>
}
#endif

#define NB_TAPS 512

void _fir(float* in, float* out, int nb_sample){
	int i, j;
	float taps[NB_TAPS];
	for(i=0; i<NB_TAPS; i++){
		taps[i] = 1.0/NB_TAPS;
	}

//	printf("out : %#x\n", out);
#ifndef DSP
	float last[NB_TAPS];

	int last_id = 0;
	memset(last,0,NB_TAPS*sizeof(float));

	for(i=0; i<nb_sample; i++){
		out[i] = 0;
		last[last_id] = in[i];
		for(j=0; j<NB_TAPS; j++){
			out[i] += taps[j]*last[(last_id+j)%NB_TAPS];
		}
		last_id = (last_id+1)%NB_TAPS;
	}
#else
	float input[4000+512-1];

	memset(input, 0, (512-1)*sizeof(float));
	memcpy(input+512-1, in, nb_sample*sizeof(float));

	DSPF_sp_fir_gen(
			input,
			taps,
			out,
			512,
			nb_sample
	);
#endif
}