/**
 * *****************************************************************************
 * Copyright or © or Copr. IETR/INSA: Maxime Pelcat, Jean-François Nezan,
 * Karol Desnos, Julien Heulot, Clément Guy, Yaset Oliva Venegas
 *
 * [mpelcat,jnezan,kdesnos,jheulot,cguy,yoliva]@insa-rennes.fr
 *
 * This software is a computer program whose purpose is to prototype
 * parallel applications.
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
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
 * knowledge of the CeCILL-C license and that you accept its terms.
 * ****************************************************************************
 */

#ifndef RADIX2_FFT_H
#define RADIX2_FFT_H

#include <lrt.h>

#define N_FCT_RADIX2_FFT 11
extern lrtFct radix2_fft_fcts[N_FCT_RADIX2_FFT];

typedef enum{
	CORE_ARM1 = 4,
	CORE_ARM0 = 5,
	CORE_DSP0 = 0,
	CORE_DSP3 = 3,
	CORE_DSP1 = 1,
	CORE_DSP2 = 2,
} PE;

typedef enum{
	CORE_TYPE_C6X = 0,
	CORE_TYPE_ARM = 1,
} PEType;

typedef enum{
	RADIX2_FFT_SRC_FCT = 0,
	RADIX2_FFT_T_FCT = 1,
	RADIX2_FFT_DFT_N2_FCT = 2,
	RADIX2_FFT_SNK_FCT = 3,
	RADIX2_FFT_CFGFFT_FCT = 4,
	RADIX2_FFT_CPLXSP_TO_CPLX16_FCT = 5,
	RADIX2_FFT_CPLX16_TO_CPLXSP_FCT = 6,
	DFT_RADIX2_GENIX_FCT = 7,
	RADIX2_STAGE_DFT_2_FCT = 8,
	RADIX2_STAGE_CFG_FCT = 9,
	RADIX2_STAGE_GENIX_FCT = 10,
} FctIxs;

#endif//RADIX2_FFT_H
