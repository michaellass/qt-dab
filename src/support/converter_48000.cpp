#
/*
 *    Copyright (C) 2011 .. 2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of Qt-DAB
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"converter_48000.h"
#include	"radio.h"
#include	<cstdio>
/*
 */
	converter_48000::converter_48000 (RadioInterface *mr,
	                                  RingBuffer<float> *outB):
	                                mapper_16 (16000, 48000, 2 * 1600),
	                                mapper_24 (24000, 48000, 2 * 2400),
	                                mapper_32 (32000, 48000, 2 * 3200),
	                                outputBuffer (outB) {
	this	-> filePointer		= nullptr;
	this    -> peakLevelCurSampleCnt        = 0;
	repetitionCounter		= 8;
        this    -> peakLevelSampleMax   = 48000 / repetitionCounter; 
        this    -> absPeakLeft          = 0.0f;
        this    -> absPeakRight         = 0.0f;
	connect (this, SIGNAL (showPeakLevel (float, float)),
	         mr, SLOT (showPeakLevel (float, float)));
}

	converter_48000::~converter_48000 () {
}

//
//	This one is a hack for handling different baudrates coming from
//	the aac decoder. call is from the GUI, triggered by the
//	aac decoder or the mp3 decoder
void	converter_48000::convert (std::complex<int16_t> *V,
	                                    int32_t amount, int32_t rate) {
	switch (rate) {
	   case 16000:	
	      convert_16000 (V, amount);
	      return;
	   case 24000:
	      convert_24000 (V, amount);
	      return;
	   case 32000:
	      convert_32000 (V, amount);
	      return;
	   default:
	   case 48000:
	      convert_48000 (V, amount);
	      return;
	}
}

void	converter_48000::start_audioDump         (SNDFILE *f) {
	locker. lock ();
	filePointer	= f;
	locker. unlock ();
}

void	converter_48000::stop_audioDump          () {
	locker. lock ();
	filePointer	= nullptr;
	locker. unlock ();
}

//
//	scale up from 16 -> 48
//	amount gives number of pairs
void	converter_48000::convert_16000	(std::complex<int16_t> *V, int amount) {
Complex buffer       [mapper_16. getOutputsize()];
int32_t	result;

	for (int i = 0; i < amount; i ++) {
	   if (mapper_16.
	            convert (Complex (real (V [i]) / 32767.0,
	                              imag (V [i]) / 32767.0),
	                                           buffer, &result)) {
	      
	      dump (buffer, result);
	      eval (buffer, result);
	      outputBuffer -> putDataIntoBuffer ((float *)buffer, 2 * result);
	   }
	}
}

//	scale up from 24000 -> 48000
//	amount gives number of pairs
void	converter_48000::convert_24000	(std::complex<int16_t> *V, int amount) {
Complex buffer	[mapper_24. getOutputsize()];
int32_t	result;

	for (int i = 0; i < amount; i ++) {
	   if (mapper_24.
	            convert (Complex (real (V [i]) / 32767.0,
	                              imag (V [i]) / 32767.0),
	                                           buffer, &result)) {
	      dump (buffer, result);
	      eval (buffer, result);
	      outputBuffer -> putDataIntoBuffer ((float *)buffer, 2 * result);
	   }
	}
}

//	scale up from 32000 -> 48000
//	amount is number of pairs
void	converter_48000::convert_32000	(std::complex<int16_t> *V, int amount) {
Complex      buffer       [mapper_32. getOutputsize()];
int32_t	result;

	for (int i = 0; i < amount; i ++) {
	   if (mapper_32.
	            convert (Complex (real (V [i]) / 32767.0,
	                              imag (V [i]) / 32767.0),
	                                           buffer, &result)) {
	      dump (buffer, result);
	      eval (buffer, result);
	      outputBuffer -> putDataIntoBuffer ((float *)buffer, 2 * result);
	   }
	}
}

void	converter_48000::convert_48000	(std::complex<int16_t> *V, int amount) {
Complex buffer [amount];

	for (int i = 0; i < amount; i ++) {
	   buffer [i]	= Complex (real (V [i]) / 32768.0, 
	                           imag (V [i]) / 32768.0);
	}
	dump (buffer, amount);
	eval (buffer, amount);
	outputBuffer -> putDataIntoBuffer (buffer, 2 * amount);
}

void	converter_48000::dump (Complex *buffer, int size) {
	locker. lock();
        if (filePointer != nullptr)
           sf_writef_float (filePointer, (float *)buffer, size);
        locker. unlock();
}

void	converter_48000::eval (Complex *buffer, int amount) {
	for (int i = 0; i < amount; i ++)
	   evaluatePeakLevel (buffer [i]);
}

void	converter_48000::evaluatePeakLevel (const Complex s) {
const float absLeft  = std::abs (real (s));
const float absRight = std::abs (imag (s));

	if (absLeft  > absPeakLeft)  
	   absPeakLeft  = absLeft;
	if (absRight > absPeakRight)
	   absPeakRight = absRight;

	peakLevelCurSampleCnt ++;
	if (peakLevelCurSampleCnt >= peakLevelSampleMax) {
	   peakLevelCurSampleCnt = 0;

	   float leftDb  = (absPeakLeft  > 0.0f ?
	                   20.0f * std::log10 (absPeakLeft)  : -40.0f);
	   float rightDb = (absPeakRight > 0.0f ?
	                   20.0f * std::log10 (absPeakRight) : -40.0f);
	   emit showPeakLevel (leftDb, rightDb);
	   absPeakLeft	= 0.0f;
	   absPeakRight = 0.0f;
	}
}
