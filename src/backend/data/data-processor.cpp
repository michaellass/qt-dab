#
/*
 *    Copyright (C) 2015 .. 2024
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the Qt-DAB
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
#include	"dab-constants.h"
#include	"radio.h"
#include	"bit-extractors.h"
#include	"crc-handlers.h"
#include	"data-processor.h"
#include	"virtual-datahandler.h"
#include	"ip-datahandler.h"
#include	"mot-handler.h"
#include	"journaline-datahandler.h"
#include	"tdc-datahandler.h"

//	\class dataProcessor
//	The main function of this class is to assemble the 
//	MSCdatagroups and dispatch to the appropriate handler
//	11-bit from HandleFIG0Extension13, see ETSI TS 101 756 table 16
//	AppType -> https://www.etsi.org/deliver/etsi_ts/101700_101799/101756/02.02.01_60/ts_101756v020201p.pdf
/*char *getUserApplicationType (int16_t appType) {
	char *buffer = (char *)malloc(30);
        switch (appType) {
           case 1:     return "Dynamic labels (X-PAD only)";
           case 2:     return "MOT Slide Show";		// ETSI TS 101 499
           case 3:     return "MOT Broadcast Web Site";
           case 4:     return "TPEG";			// ETSI TS 103 551
           case 5:     return "DGPS";
           case 6:     return "TMC";
           case 7:     return "SPI, was EPG";		// ETSI TS 102 818
           case 8:     return "DAB Java";
           case 9:     return "DMB";			// ETSI TS 102 428
           case 0x00a: return "IPDC services";
           case 0x00b: return "Voice applications";
           case 0x00c: return "Middleware";
           case 0x00d: return "Filecasting";		// ETSI TS 102 979
           case 0x44a: return "Journaline";
           default:
	       sprintf(buffer, "(0x%04x)", appType);;
	       return buffer;
        }
        return "";
} */

#define	RSDIMS	12
#define	FRAMESIZE 188
//	fragmentsize == Length * CUSize
	dataProcessor::dataProcessor	(RadioInterface *mr,
	                                 packetdata	*pd,
	                                 RingBuffer<uint8_t> *dataBuffer,
	                                 bool	backgroundFlag):
	                                     my_rsDecoder (8, 0435, 0, 1, 16) {
	this	-> myRadioInterface	= mr;
	this	-> bitRate		= pd -> bitRate;
	this	-> DSCTy		= pd -> DSCTy;
	this	-> appType		= pd -> appType;
	this	-> packetAddress	= pd -> packetAddress;
	this	-> DGflag		= pd -> DGflag;
	this	-> FEC_scheme		= pd -> FEC_scheme;
	this	-> dataBuffer		= dataBuffer;

	AppVector. resize (RSDIMS * FRAMESIZE);
	FECVector. resize (9 * 22);
	for (int i = 0; i < 9; i ++)
	   FEC_table [i] = false;

	fillPointer	= 0;
	fprintf (stderr, "** DBG: dataProcessor: appType=%d FEC=%d DSCTy=%d (", pd -> appType, FEC_scheme, pd -> DSCTy);
	switch (DSCTy) {
	   default:
	      fprintf (stderr, "DSCTy %d not supported\n", DSCTy);
	      my_dataHandler	= new virtual_dataHandler();
	      break;

	   case 5:
	      if (appType == 0x44a)
	         my_dataHandler	= new journaline_dataHandler();
	      else
	         my_dataHandler	= new tdc_dataHandler (mr, dataBuffer, appType);
	      break;

	   case 44:
	      fprintf (stderr, "going to install gournaline\n");
	      my_dataHandler	= new journaline_dataHandler();
	      break;

	   case 59:
	      my_dataHandler	= new ip_dataHandler (mr, dataBuffer);
	      break;

	   case 60:
	      my_dataHandler	= new motHandler (mr, backgroundFlag);
	      break;
	   
	}

	packetState	= 0;
}

	dataProcessor::~dataProcessor() {
	delete		my_dataHandler;
}

void	dataProcessor::addtoFrame (std::vector<uint8_t>  outV) {
//	There is - obviously - some exception, that is
//	when the DG flag is on and there are no datagroups for DSCTy5

	if ((this -> DSCTy == 5) &&
	    (this -> DGflag))	// no datagroups
	      handleTDCAsyncstream (outV. data (), 24 * bitRate);
	   else
	      handlePackets (outV. data (), 24 * bitRate);
}
//
void	dataProcessor::handlePackets (uint8_t *dataL, int16_t length) {
uint8_t *data = dataL;
	while (true) {
//	pLength is in bits
	   int32_t pLength = (getBits_2 (data, 0) + 1) * 24 * 8;
	   if (length < pLength)	// be on the safe side
	      return;

	   if (!FEC_scheme) 
	      handlePacket (data);
	   else
	      handleRSPacket (data);
//
//	prepare for the next round
	   length -= pLength;
	   if (length < 24) {
	      return;
	   }
	   data = &(data [pLength]);
	}
}

void	dataProcessor::handlePacket (uint8_t *vec) {
static int expected_cntidx = 0;
static bool assembling		= false;
	uint8_t Length	= (getBits (vec, 0, 2) + 1) * 24;
	if (!check_CRC_bits (vec, Length * 8)) {
	   fprintf (stderr, "crc fails\n");
	   return;
	}

//	Continuity index:
	uint8_t cntIdx	= getBits (vec, 2, 2);
//	First/Last flag:
	uint8_t flflg	= getBits (vec, 4, 2);
//	Packet address
	uint16_t paddr	= getBits (vec, 6, 10);
//	Useful data length
	uint8_t udlen	= getBits (vec, 17,7); 
	if (udlen == 0)
	   return;

//	if (cntIdx != expected_cntidx) {
//	   expected_cntidx = 0;
//	   return;
//	}
//	expected_cntidx = (cntIdx + 1) % 4;

	switch (flflg) {
	   case 2:  // First data group packet
	      series. resize (udlen * 8);
	      for (uint16_t i = 0; i < udlen * 8; i ++)
	         series [i] = vec [3 * 8 + i];
	      assembling	= true;
//	fprintf (stderr, "start assembling\n");
	      return;

	   case 0:    // Intermediate data group packet
	      if (assembling) {
	         int currentLength = series. size ();
	         if (currentLength + udlen * 8 > 4 * 8192) {
	            assembling = false;
//	            fprintf (stderr, "too large???\n");
	            return;
	         }
	         series. resize (currentLength + udlen * 8);
	         for (int i = 0; i < udlen * 8; i ++) 
	            series [currentLength + i] = vec [3 * 8 + i];
	      }
	      return;

	   case 1:  // Last data group packet
	      if (assembling) {
	         int currentLength = series. size ();
	         if (currentLength + udlen * 8 > 4 * 8192) {
	            assembling = false;
//	            fprintf (stderr, "too large???\n");
	            return;
	         }
	         series. resize (currentLength + udlen * 8);
	         for (int i = 0; i < udlen * 8; i ++)
	            series [currentLength + i] = vec [3 * 8 + i];
	         assembling = false;
//	      fprintf (stderr, "to datahandler\n");
	         my_dataHandler	-> add_mscDatagroup (series);
	         series. resize (0);
	      }
	      return;

	      case 3: { // Single packet, mostly padding
	         if (paddr == 2) {
	            }
              }
	      break;
	   }
}
//
//	we try to ensure that when the RS packages are readin, we
//	have exactly RSDIMS * FRAMESIZE uint's read
void	dataProcessor::handleRSPacket (uint8_t  *vec) {
int32_t pLength		= (getBits_2 (vec, 0) + 1) * 24;
uint16_t address	= getBits (vec, 6, 10);

	if ((pLength == 24) && (address == 1022)) {
	   if (fillPointer < RSDIMS * FRAMESIZE) {
	      fillPointer = 0;
	      return;
	   }
	   uint8_t counter = getBits (vec, 2, 4);
	   registerFEC (vec, counter);
	   if (FEC_complete ()) {
	      processRS (AppVector, FECVector);
	      handle_RSpackets (AppVector);
	      fillPointer = 0;
	   }
	}
	else {
	   if (fillPointer >= RSDIMS * FRAMESIZE)
	      return;
	   fillPointer = addPacket (vec, AppVector, fillPointer);
	   if (fillPointer >= RSDIMS * FRAMESIZE)
	      clear_FECtable ();
	}
}

void	dataProcessor::clear_FECtable () {
	for (int i = 0; i < 9; i ++)
	   FEC_table [i] = false;
}
//
//	addPacket basically packs the sequence of bits into a sequence
//	of bytes, for processing by the RS decoder
int	dataProcessor::addPacket (uint8_t *vec,
	                          std::vector<uint8_t> &theBuffer,
	                          int fillPointer) {
	int16_t	packetLength	= (getBits_2 (vec, 0) + 1) * 24;

	if (fillPointer + packetLength > RSDIMS * FRAMESIZE)
	   fprintf (stderr, " no match %d %d\n", fillPointer, packetLength);
	for (int i = 0; i < packetLength; i ++) {
	   uint8_t temp = 0;
	   for (int j = 0; j < 8; j ++)
	      temp = (temp << 1) | (vec [i * 8 + j] == 0 ? 0 : 1);
	   theBuffer [fillPointer + i] = temp;
	}
	return fillPointer + packetLength;
}
//
//	The output of the RS decoding is a vector with a sequence
//	of packets, first dispatch and separate the packet sequence
//	into its elements
//
void	dataProcessor::handle_RSpackets (std::vector<uint8_t> &vec) {
	for (int baseP = 0; baseP < RSDIMS * FRAMESIZE; ) {
	   int16_t packetLength = (((vec [baseP] & 0xc0) >> 6) + 1) * 24;
	   handle_RSpacket (&(vec. data ()) [baseP], packetLength);
	   baseP += packetLength;
	}
}

static
uint8_t bitList [] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
//
//	The RS data is with packed bytes, while the basis infrastructure
//	is with bit sequences, so to keep things simple, we just
//	transform the byte sequence into a bit sequence
void	dataProcessor::handle_RSpacket (uint8_t *packet, int16_t packetLength) {
std::vector<uint8_t> bitData (packetLength * 8);
	for (int i = 0; i < packetLength; i ++) {
	   uint8_t temp = packet [i];
	   for (int j = 0; j < 8; j ++) {
	      uint8_t theBit = (temp & bitList [j]) == 0 ? 0 : 1;
	      bitData [8 * i + j] = theBit;
	   }
	}
	handlePacket (bitData. data ());
}
//
//	as it tuns out, the FEC data packages are arriving in order,
//	so it would have been sufficient just to wait until the
//	package with counter '8' was seen
void	dataProcessor::registerFEC (uint8_t *vec, int cnt) {
	for (int i = 0; i < 22; i ++) {
	   uint8_t temp = 0;
	   for (int j = 0; j < 8; j ++)
	      temp = (temp << 1) | vec [16 + 8 * i + j];
	   FECVector [cnt * 22 + i] = temp;
	}
	FEC_table [cnt] = true;
}

bool	dataProcessor::FEC_complete () {
	for (int i = 0; i < 9; i ++)
	   if (!FEC_table [i])
	      return false;
	return true;
}
//
//
//	Really no idea what to do here
void	dataProcessor::handleTDCAsyncstream (const uint8_t *data,
	                                              int32_t length) {
int16_t	packetLength	= (getBits_2 (data, 0) + 1) * 24;
int16_t	continuityIndex	= getBits_2 (data, 2);
int16_t	firstLast	= getBits_2 (data, 4);
int16_t	address		= getBits   (data, 6, 10);
uint16_t command	= getBits_1 (data, 16);
int16_t	usefulLength	= getBits_7 (data, 17);

	(void)	length;
	(void)	packetLength;
	(void)	continuityIndex;
	(void)	firstLast;
	(void)	address;
	(void)	command;
	(void)	usefulLength;
	if (!check_CRC_bits (data, packetLength * 8))
	   return;
}
//
//	To keep things simple, we abstraxct from the rs decoding
//	by providing - as separate vectors - the RSDIMS * FRAMESIZE
//	app data values and the 9 * 22 RS data values
//	The appData vector is overwritten with the corrected data
//
void	dataProcessor::processRS (std::vector<uint8_t> &appData,
	                          const std::vector<uint8_t> &RSdata) {
static
uint8_t table [RSDIMS][FRAMESIZE + 16];
uint8_t rsOut	[FRAMESIZE];
//	Assert appdata . size () == RSDIMS * FRAMESIZE
//	Assert RSdata. size () == 9 * 22;
	for (int i = 0; i < RSDIMS * FRAMESIZE; i ++)
	   table [i % RSDIMS][i / RSDIMS] = appData [i];
	for (int i = 0; i <  (int)(RSdata. size ()); i ++)
	   table [i % RSDIMS] [FRAMESIZE + i / RSDIMS] = RSdata [i];

	for (int i = 0; i < RSDIMS; i ++) {
	   int xx = my_rsDecoder. dec (table [i], rsOut, 51);
//	   fprintf (stderr, "rs decoder says %d\n", xx);
	   for (int j = 0; j < FRAMESIZE; j ++)
	      table [i][j] = rsOut [j];
	}
//
//	copy the table back to the vector
	for (int i = 0; i < RSDIMS * FRAMESIZE; i ++)
	   appData [i] = table [i % RSDIMS][i / RSDIMS];
}

