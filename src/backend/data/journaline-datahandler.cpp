# /*
 *    Copyright (C) 2015 .. 2024
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
#include	"journaline-datahandler.h"
#include	"dabdatagroupdecoder.h"
#include	"bit-extractors.h"

#include	<sys/time.h>
#include	"newsobject.h"
#include	"NML.h"
static
void my_callBack (
    const DAB_DATAGROUP_DECODER_msc_datagroup_header_t *header,
    const unsigned long len,
    const unsigned char *buf,
    void *arg) {
	struct timeval theTime;
	gettimeofday (&theTime, NULL);
	(void) header;
	unsigned char buffer [4092];
	(void) arg;
	long unsigned int nmlSize	= 0;
	NML::RawNewsObject_t theBuffer;
	NewsObject hetNieuws (len, buf, &theTime);
	hetNieuws. copyNml (&nmlSize, buffer);
	theBuffer. nml_len	= nmlSize;
	theBuffer. extended_header_len = 0;
	for (int i = 0; i < len; i ++)
	   theBuffer. nml [i] = buffer [i];
	RemoveNMLEscapeSequences theRemover;
	NMLFactory xxx;
	NML *ttt = xxx.CreateNML (theBuffer, &theRemover);
//	add_to_dataBase (ttt);
	delete ttt;
}

	journaline_dataHandler::journaline_dataHandler() {
	theDecoder	= DAB_DATAGROUP_DECODER_createDec (my_callBack, this);
	init_dataBase ();
}

	journaline_dataHandler::~journaline_dataHandler() {
	DAB_DATAGROUP_DECODER_deleteDec (theDecoder);
	destroy_dataBase ();
}

//void	journaline_dataHandler::add_mscDatagroup (QByteArray &msc) {
void	journaline_dataHandler::add_mscDatagroup (std::vector<uint8_t> msc) {
int16_t	len	= msc. size ();
uint8_t	*data	= (uint8_t *)(msc. data());
uint8_t	*buffer	= (uint8_t *) alloca (len / 8 * sizeof (uint8_t));
int32_t	res;
	for (uint16_t i = 0; i < len / 8; i ++)
	   buffer [i] = getBits (data, 8 * i, 8);

	res = DAB_DATAGROUP_DECODER_putData (theDecoder, len / 8, buffer);
	if (res < 0)
	   return;
}

void	journaline_dataHandler::init_dataBase 	() {
}

void	journaline_dataHandler::destroy_dataBase	() {
}

void	journaline_dataHandler::add_to_dataBase (NML * NMLelement) {

//	if (ttt -> isRootObject ()) {
//	   fprintf (stderr, "\n\nRoot: the Title is %s with %d objectId\n",
//                      ttt -> GetTitle (). c_str (),
//                      ttt -> GetObjectId ());
//           std::vector<NML::Item_t> theItems = ttt -> GetItems ();
//           for (auto &theItem : theItems)
//              fprintf (stderr, "%d : %s\n", theItem. link_id,
//                                            theItem. text. c_str());
//	}
//	else
//	if (ttt -> isMenu ()) {
//	   fprintf (stderr, "\n\nMenu: the Title is %s with %d objectId\n",
//	              ttt -> GetTitle (). c_str (),
//	              ttt -> GetObjectId ());
//	   std::vector<NML::Item_t> theItems = ttt -> GetItems ();
//	   for (auto &theItem : theItems)
//	      fprintf (stderr, "%d : %s\n", theItem. link_id,
//	                                    theItem. text. c_str());
//	}
//	else {
//	   std::vector<NML::Item_t> theItems = ttt -> GetItems ();
//	   fprintf (stderr, "Text or List Object %d\n",
//	                                    ttt -> GetObjectId ());
//            for (auto &theItem : theItems)
//               fprintf (stderr, "%d : %s\n", theItem. link_id,
//                                          theItem. text. c_str());
//	}
}

