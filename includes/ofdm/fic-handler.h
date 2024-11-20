#
/*
 *    Copyright (C) 2016 .. 2023
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
#
/*
 * 	FIC data
 */
#pragma once

#include	<QObject>
#include	<QMutex>
#include	<cstdio>
#include	<cstdint>
#include	<vector>
#include	<atomic>
#include	"viterbi-spiral.h"
#include	"dab-params.h"
#include	"fib-decoder.h"


class	RadioInterface;
class	dabParams;

class ficHandler: public fibDecoder {
Q_OBJECT
public:
		ficHandler		(RadioInterface *, uint8_t);
		~ficHandler		();
	void	process_ficBlock	(std::vector<int16_t> &, int16_t);
	void	stop			();
	void	restart			();
	void	start_ficDump		(FILE *);
	void	stop_ficDump		();
	void	get_fibBits		(uint8_t *, bool *);
	int	get_ficQuality		();
private:
	dabParams	params;
	viterbiSpiral	myViterbi;
	uint8_t		bitBuffer_out	[768];
        int16_t		ofdm_input	[2304];
	bool		punctureTable	[3072 + 24];
	uint8_t		fibBits		[4 * 768];
	bool		ficValid	[4];
	uint8_t		ficBuffer	[256];
	uint8_t		PRBS		[768];
	void		process_ficInput	(int16_t, bool *);
	int16_t		index;
	int16_t		BitsperBlock;
	int16_t		ficno;
	int16_t		ficBlocks;
	int16_t		ficMissed;
	int16_t		ficRatio;
	uint16_t	convState;
	FILE		*ficDumpPointer;
	QMutex          ficLocker;
	int		ficPointer;
	std::atomic<bool> running;
	int		successRatio;
	int		fibCounter;
signals:
	void		show_ficQuality	(int, int);
};



