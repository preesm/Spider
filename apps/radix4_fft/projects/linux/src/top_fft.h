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

#ifndef TOP_FFT_H
#define TOP_FFT_H

#include <platform.h>
#include <lrt.h>

#define N_FCT_TOP_FFT 12
extern lrtFct top_fft_fcts[N_FCT_TOP_FFT];

typedef enum{
	CORE_CORE3 = 0,
	CORE_CORE0 = 1,
	CORE_CORE2 = 2,
	CORE_CORE1 = 3,
	CORE_DSP0 = 4,
} PE;

typedef enum{
	CORE_TYPE_X86 = 0,
} PEType;

typedef enum{
	SRC_FCT = 0,
	SNK_FCT = 1,
	CONFIGFFT_FCT = 2,
	ORDERING_FCT = 3,
	MONOFFT_FCT = 4,
	GENSWITCHSEL_FCT = 5,
	SELCFG_FCT = 6,
	END0_FCT = 7,
	END1_FCT = 8,
	CFGFFTSTEP_FCT = 9,
	FFT_RADIX2_FCT = 10,
	GENIX_FCT = 11,
} FctIxs;

#endif//TOP_FFT_H
