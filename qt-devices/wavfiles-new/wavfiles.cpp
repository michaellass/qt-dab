#
/*
 *    Copyright (C) 2013 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the Qt-DAB program
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
#include	<QString>
#include	<QFileDialog>
#include	<cstdio>
#include	<unistd.h>
#include	<cstdlib>
#include	<fcntl.h>
#include	<sys/time.h>
#include	<ctime>
#include	"wavfiles.h"
#include	"device-exceptions.h"

#define	__BUFFERSIZE__	8 * 32768

	wavFiles::wavFiles (): _I_Buffer (__BUFFERSIZE__) {
SF_INFO *sf_info;

	setupUi (&myFrame);
	myFrame. show	();
	fileName	= getFileName ();
	if (fileName == "")
	   throw device_exception ("no file specified");

	sf_info		= (SF_INFO *)alloca (sizeof (SF_INFO));
	sf_info	-> format	= 0;
	filePointer	= sf_open (fileName. toUtf8(). data(),
	                                          SFM_READ, sf_info);
	if (filePointer == nullptr) {
	   throw device_exception (fileName. toStdString () + "no sdr file");
	}
	if ((sf_info -> samplerate != INPUT_RATE) ||
	    (sf_info -> channels != 2)) {
	   sf_close (filePointer);
	   throw device_exception (fileName. toStdString () + "wrong samplerate");
	}
	nameofFile	-> setText (fileName);
	fileProgress	-> setValue (0);
	currentTime	-> display (0);
	int64_t fileLength	= sf_seek (filePointer, 0, SEEK_END);
	totalTime	-> display ((float)fileLength / INPUT_RATE);
	running. store (false);
}
//
//	Note that running == true <==> readerTask has value assigned

	wavFiles::~wavFiles	() {
	if (running. load()) {
	   readerTask	-> stopReader();
	   while (readerTask -> isRunning())
	      usleep (500);
	   delete readerTask;
	}
	if (filePointer != nullptr)
	   sf_close (filePointer);
}

bool	wavFiles::restartReader		(int32_t freq) {
	(void)freq;
	if (running. load())
           return true;
        readerTask      = new wavReader (this, filePointer, &_I_Buffer);
        running. store (true);
        return true;
}

void	wavFiles::stopReader() {
       if (running. load()) {
           readerTask   -> stopReader();
           while (readerTask -> isRunning())
              usleep (100);
	   delete readerTask;
        }
        running. store (false);
}

//	size is in I/Q pairs
int32_t	wavFiles::getSamples	(std::complex<float> *V, int32_t size) {
int32_t	amount;
	
	if (filePointer == nullptr)
	   return 0;

	while (_I_Buffer. GetRingBufferReadAvailable() < size)
	      usleep (100);

	amount = _I_Buffer. getDataFromBuffer (V, size);
	
	return amount;
}

int32_t	wavFiles::Samples() {
	return _I_Buffer. GetRingBufferReadAvailable();
}

void    wavFiles::setProgress (int progress, float timelength) {
        fileProgress      -> setValue (progress);
        currentTime       -> display (timelength);
}

bool	wavFiles::isFileInput	() {
	return true;
}

QString	wavFiles::getFileName 	() {

	QString file = QFileDialog::getOpenFileName (nullptr,
	                                             "Open file ...",
	                                             QDir::homePath(),
	                                             "raw data (*.sdr)");
	      if (file == QString (""))
	         return "";

	      return QDir::toNativeSeparators (file);
}

