/*
 *    Copyright (C) 2014 .. 2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the Qt-DAB program
 *
 *    Qt-DAB is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    Qt-DAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Qt-DAB; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include	"tii-detector-1.h"
#include        "settingNames.h"
#include        "settings-handler.h"
#include	<complex>

// TII pattern for transmission modes I, II and IV
static const uint8_t table[] = {
	0x0f,	// 0 0 0 0 1 1 1 1		0
	0x17,	// 0 0 0 1 0 1 1 1		1
	0x1b,	// 0 0 0 1 1 0 1 1		2
	0x1d,	// 0 0 0 1 1 1 0 1		3
	0x1e,	// 0 0 0 1 1 1 1 0		4
	0x27,	// 0 0 1 0 0 1 1 1		5
	0x2b,	// 0 0 1 0 1 0 1 1		6
	0x2d,	// 0 0 1 0 1 1 0 1		7
	0x2e,	// 0 0 1 0 1 1 1 0		8
	0x33,	// 0 0 1 1 0 0 1 1		9
	0x35,	// 0 0 1 1 0 1 0 1		10
	0x36,	// 0 0 1 1 0 1 1 0		11
	0x39,	// 0 0 1 1 1 0 0 1		12
	0x3a,	// 0 0 1 1 1 0 1 0		13
	0x3c,	// 0 0 1 1 1 1 0 0		14
	0x47,	// 0 1 0 0 0 1 1 1		15
	0x4b,	// 0 1 0 0 1 0 1 1		16
	0x4d,	// 0 1 0 0 1 1 0 1		17
	0x4e,	// 0 1 0 0 1 1 1 0		18
	0x53,	// 0 1 0 1 0 0 1 1		19
	0x55,	// 0 1 0 1 0 1 0 1		20
	0x56,	// 0 1 0 1 0 1 1 0		21
	0x59,	// 0 1 0 1 1 0 0 1		22
	0x5a,	// 0 1 0 1 1 0 1 0		23
	0x5c,	// 0 1 0 1 1 1 0 0		24
	0x63,	// 0 1 1 0 0 0 1 1		25
	0x65,	// 0 1 1 0 0 1 0 1		26
	0x66,	// 0 1 1 0 0 1 1 0		27
	0x69,	// 0 1 1 0 1 0 0 1		28
	0x6a,	// 0 1 1 0 1 0 1 0		29
	0x6c,	// 0 1 1 0 1 1 0 0		30
	0x71,	// 0 1 1 1 0 0 0 1		31
	0x72,	// 0 1 1 1 0 0 1 0		32
	0x74,	// 0 1 1 1 0 1 0 0		33
	0x78,	// 0 1 1 1 1 0 0 0		34
	0x87,	// 1 0 0 0 0 1 1 1		35
	0x8b,	// 1 0 0 0 1 0 1 1		36
	0x8d,	// 1 0 0 0 1 1 0 1		37
	0x8e,	// 1 0 0 0 1 1 1 0		38
	0x93,	// 1 0 0 1 0 0 1 1		39
	0x95,	// 1 0 0 1 0 1 0 1		40
	0x96,	// 1 0 0 1 0 1 1 0		41
	0x99,	// 1 0 0 1 1 0 0 1		42
	0x9a,	// 1 0 0 1 1 0 1 0		43
	0x9c,	// 1 0 0 1 1 1 0 0		44
	0xa3,	// 1 0 1 0 0 0 1 1		45
	0xa5,	// 1 0 1 0 0 1 0 1		46
	0xa6,	// 1 0 1 0 0 1 1 0		47
	0xa9,	// 1 0 1 0 1 0 0 1		48
	0xaa,	// 1 0 1 0 1 0 1 0		49
	0xac,	// 1 0 1 0 1 1 0 0		50
	0xb1,	// 1 0 1 1 0 0 0 1		51
	0xb2,	// 1 0 1 1 0 0 1 0		52
	0xb4,	// 1 0 1 1 0 1 0 0		53
	0xb8,	// 1 0 1 1 1 0 0 0		54
	0xc3,	// 1 1 0 0 0 0 1 1		55
	0xc5,	// 1 1 0 0 0 1 0 1		56
	0xc6,	// 1 1 0 0 0 1 1 0		57
	0xc9,	// 1 1 0 0 1 0 0 1		58
	0xca,	// 1 1 0 0 1 0 1 0		59
	0xcc,	// 1 1 0 0 1 1 0 0		60
	0xd1,	// 1 1 0 1 0 0 0 1		61
	0xd2,	// 1 1 0 1 0 0 1 0		62
	0xd4,	// 1 1 0 1 0 1 0 0		63
	0xd8,	// 1 1 0 1 1 0 0 0		64
	0xe1,	// 1 1 1 0 0 0 0 1		65
	0xe2,	// 1 1 1 0 0 0 1 0		66
	0xe4,	// 1 1 1 0 0 1 0 0		67
	0xe8,	// 1 1 1 0 1 0 0 0		68
	0xf0	// 1 1 1 1 0 0 0 0		69
};

static const int8_t Reftable[] = {
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,
	 2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,  2,  0,  0,  0,
	-1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1,
	 0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,
	 1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1
};

#define NUM_GROUPS      8
#define GROUPSIZE       24
constexpr float F_DEG_PER_RAD = (float)(180.0 / M_PI);

	TII_Detector_A::TII_Detector_A (uint8_t dabMode, 
	                          QSettings	*dabSettings):
	                               params(dabMode),
		                       T_u (params.get_T_u()),
  		                       carriers (params. get_carriers ()),
	                               my_fftHandler (params. get_T_u (),
                                                            false) {

	this    -> dabSettings  = dabSettings;
        theBuffer. resize       (T_u);
        window. resize          (T_u);
        for (int i = 0; i < T_u; i ++) {
           window [i] = 0.54 - 0.46 * cos (2 * M_PI * (DABFLOAT)i / T_u);
	   theBuffer [i] = Complex (0, 0);
	}
        memset (invTable, 0x377, 256);
        for (int i = 0; i < 70; ++i)
            invTable [table [i]] = i;
}

	TII_Detector_A::~TII_Detector_A () { }

void	TII_Detector_A::setMode	(bool b) {
	(void)b;
}

void	TII_Detector_A::resetBuffer	() {
	for (int i = 0; i < T_u; i++)
	   theBuffer[i] = Complex (0, 0);
}

void	TII_Detector_A::reset	() {
	resetBuffer ();
	for (int i = 0; i < carriers / 2; i++)
	   decodedBuffer [i] = Complex (0, 0);
}

void	TII_Detector_A::addBuffer (std::vector<Complex> v) {
	for (int i = 0; i < T_u; i ++)
	   v [i] = v [i] *  window [i];
	my_fftHandler. fft (v);
	for (int i = 0; i < T_u; i ++)
	   theBuffer [i] += v [i];
}

//	we map the "K" carriers (complex values) onto
//	a collapsed vector of "K / 8" length,
//	considered to consist of 8 segments of 24 values
//	Each "value" is the sum of 4 pairs of subsequent carriers,
//	taken from the 4 quadrants -768 .. 385, 384 .. -1, 1 .. 384, 385 .. 768
void	TII_Detector_A::collapse (Complex *inVec,
	                          Complex *etsiVec, Complex *nonetsiVec) {
Complex buffer [carriers / 2];
int	teller	= 0;
//
//	we reduce the 1536 carriers to 768 "doubles"
	for (int index = -carriers / 2; index < carriers / 2; index += 2) {
	   int fft_ind = index < 0 ? index + T_u : index + 1;
		buffer [teller ++] += inVec [fft_ind] *
	                              conj (inVec [fft_ind + 1]);
	}

//	first we compute the max
	for (int i = 0; i < 192; i++) {
	   float x [4];
	   float max = 0;
	   float sum = 0;
	   int index = 0;
	   for (int j = 0; j < 4; j++) {
	      x [j] = abs (buffer [i + j * 192]);
	      sum += x [j];
	      if (x [j] > max) {
	         max = x [j];
	         index = j;
	      }
	   }

	   float min = (sum - max) / 3;
	   if (sum < max * 1.5 && max > 0.0) {
	      buffer [i + index * 192] *= min / max;
	   }
	}

	for (int i = 0; i < 192; i++) {
	   etsiVec [i]		= Complex (0, 0);
	   nonetsiVec [i]	= Complex (0, 0);
	   for (int j = 0; j < 4; j++) {
	      Complex x		= buffer [i + j * 192];
	      etsiVec [i]	+= x;
	      nonetsiVec [i] +=
	           std::polar ((float)abs (x),
	                       (float)(arg(x) - (float)Reftable [i + j * 192]*M_PI_2));
	   }
	}
}

//	We determine first the offset of the "best fit",
//	the offset indicates the subId
static uint8_t bits [] = {0x80, 0x40, 0x20, 0x10 , 0x08, 0x04, 0x02, 0x01};

// Sort the elemtnts according to their strength
static	int fcmp (const void *a, const void *b) {
tiiResult *element1 = (tiiResult*)a;
tiiResult *element2 = (tiiResult*)b;

	if (element1 -> strength > element2 -> strength)
	   return -1;
	else
	if (element1 -> strength < element2 -> strength)
	   return 1;
	else
	   return 0;
}

std::vector<tiiResult> TII_Detector_A::processNULL (bool dxMode) {
// collapsed ETSI float values
float etsi_floatTable [NUM_GROUPS * GROUPSIZE];	

// collapsed non-ETSI float values
float nonetsi_floatTable [NUM_GROUPS * GROUPSIZE];

// collapsed ETSI complex values
Complex etsiTable [NUM_GROUPS * GROUPSIZE];		

// collapsed non-ETSI complex values
Complex nonetsiTable[NUM_GROUPS * GROUPSIZE];

// abs value of the strongest carrier
float max = 0;
// noise level
float noise = 1e9;
// results
std::vector<tiiResult> theResult;
// threshold above noise
int Threshold_db        = value_i (dabSettings, DAB_GENERAL,
                                              "tiiThreshold", 4);
//float threshold = pow (10, (float)Threshold_db / 10);
float threshold = 10;

	collapse (theBuffer. data (), etsiTable, nonetsiTable);

//	fill the float tables, determine the abs value of
//	the strongest carrier
	for (int i = 0; i < NUM_GROUPS * GROUPSIZE; i++) {
	   float x = abs (etsiTable [i]);
	   etsi_floatTable [i] = x;
	   if (x > max)
	      max = x;
	   x = abs (nonetsiTable [i]);
	   nonetsi_floatTable [i] = x;
	   if (x > max)
	      max = x;
	}

//	determine the noise level of the lowest group
	for (int subId = 0; subId < GROUPSIZE; subId++) {
	   float avg = 0;
	   for (int i = 0; i < NUM_GROUPS; i++)
	      avg += etsi_floatTable [subId + i * GROUPSIZE];
	   avg /= NUM_GROUPS;
	   if (avg < noise)
	      noise = avg;
	}

	for (int subId = 0; subId < GROUPSIZE; subId++) {
	   tiiResult element;
	   Complex sum		= Complex (0, 0);
	   Complex etsi_sum	= Complex (0, 0);
	   Complex nonetsi_sum	= Complex (0, 0);
	   int count		= 0;
	   int etsi_count	= 0;
	   int nonetsi_count	= 0;
	   int pattern		= 0;
	   int etsi_pattern	= 0;
	   int nonetsi_pattern	= 0;
	   int mainId		= 0;
	   bool norm		= false;
	   Complex *cmplx_ptr	= nullptr;
	   float *float_ptr	= nullptr;

//	The number of times the limit is reached in the group is counted
	   for (int i = 0; i < NUM_GROUPS; i++) {
	      if (etsi_floatTable [subId + i * GROUPSIZE] >
	                                      noise * threshold) {
	         etsi_count++;
	         etsi_pattern |= (0x80 >> i);
	         etsi_sum += etsiTable[subId + GROUPSIZE * i];
	      }
	      if (nonetsi_floatTable[subId + i * GROUPSIZE] >
	                                          noise * threshold) {
	         nonetsi_count++;
	         nonetsi_pattern |= (0x80 >> i);
	         nonetsi_sum += nonetsiTable[subId + GROUPSIZE * i];
	      }
	   }

	   if ((etsi_count >= 4) || (nonetsi_count >= 4)) {
	      if (abs (nonetsi_sum) > abs (etsi_sum)) {
	         norm		= true;
	         sum		= nonetsi_sum;
	         cmplx_ptr	= nonetsiTable;
	         float_ptr	= nonetsi_floatTable;
	         count		= nonetsi_count;
	         pattern	= nonetsi_pattern;
	      }
	      else {
	         sum		= etsi_sum;
	         cmplx_ptr	= etsiTable;
	         float_ptr	= etsi_floatTable;
	         count		= etsi_count;
	         pattern	= etsi_pattern;
	      }
	   }

//	Find the Main Id that matches the pattern
	   if (count == 4) {
	      for (; mainId < (int)sizeof(table); mainId++)
	         if (table[mainId] == pattern)
	            break;
	   }

//	Find the best match. We extract the four max values as bits
	   else
	   if (count > 4) {
	      float mm = 0;
	      for (int k = 0; k < (int)sizeof(table); k++) {
	         Complex val = Complex (0,0);
	         for (int i = 0; i < NUM_GROUPS; i++) {
	            if (table[k] & bits[i])
	               val += cmplx_ptr[subId + GROUPSIZE * i];
	         }
	         if (abs (val) > mm) {
	            mm = abs (val);
	            sum = val;
	            mainId = k;
	         }
	      }
	   }
// List the result
	   if (count >= 4) {
	      element. mainId		= mainId;
	      element. subId		= subId;
	      element. strength		= abs (sum)/max/4;
	      element. phase		= arg(sum) * F_DEG_PER_RAD;
	      element. norm		= norm;
	      theResult. push_back (element);
	   }

// Calculate the level of the second main ID
//	   for (int i = 0; i < NUM_GROUPS; i++) {
//	      if ((table [mainId] & bits [i])== 0) {
//	         int index = subId + GROUPSIZE * i;
//	         if (float_ptr [index] > noise * threshold)
//	            sum += cmplx_ptr [index];
//	      }
//	   }
// List only additional main ID 99
//	   element. mainId	= 99;
//	   element. strength	= abs (sum)/max/(count - 4);
//	   element. phase	= arg(sum) * F_DEG_PER_RAD;
//	   element. norm	= norm;
//	   theResult. push_back (element);
	}
//	fprintf (stderr, "max =%.0f, noise = %.1fdB\n",
//	                                  max, 10 * log10 (noise/max));
	if (max > 4000000)
	   for (int i = 0; i < carriers / 2; i++)
	      decodedBuffer [i] *= 0.9;
	resetBuffer ();
	qsort (theResult.data (), theResult. size (),
	                               sizeof (tiiResult), &fcmp);

	return theResult;
}
