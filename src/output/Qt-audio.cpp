#
/*
 *    Copyright (C) 2014 .. 2023
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

#include	<stdio.h>
#include	"Qt-audio.h"
#include	<QSettings>
#include	"settingNames.h"
#include	"settings-handler.h"

#if QT_VERSION > QT_VERSION_CHECK (6, 0, 0)
	Qt_Audio::Qt_Audio (QSettings *settings):
	                    tempBuffer (16 * 32768) { 
	audioSettings		= settings;
	working. store		(false);
	newDeviceIndex		= -1;
	
	m_audioOutput		= nullptr;

	m_settings. setChannelCount (2);
	m_settings. setSampleRate (48000);
	m_settings. setSampleFormat (QAudioFormat::Float);
	m_settings. setChannelConfig (QAudioFormat::ChannelConfigStereo);

	const QAudioDevice &defaultDevice =
	                       QMediaDevices::defaultAudioOutput ();
	outputDevices. push_back (defaultDevice);

	for (auto &theDevice : QMediaDevices::audioOutputs ()) {
	   if (theDevice. isFormatSupported (m_settings))
	   if (theDevice. isFormatSupported (m_settings)) {
	      outputDevices. push_back (theDevice);
	   }
	}
}

	Qt_Audio::~Qt_Audio () {
}

QStringList	Qt_Audio::streams	() {
QStringList nameList;
	for (auto & listEl: outputDevices)
	   nameList << listEl. description ();
	return nameList;
}
//
//	Note that - by convention - all audio samples here
//	are in a rate 48000
void	Qt_Audio::audioOutput (float *fragment, int32_t size) {
	if (!working. load ())
	   return;
	int aa = tempBuffer. GetRingBufferWriteAvailable ();
	aa	= std::min ((int)(size * sizeof (float)), aa);
	aa	&= ~03;
	tempBuffer. putDataIntoBuffer ((char *)fragment, aa);
	
	int periodSize = 1 * 480 * sizeof (float);	// 10 msec
	char buffer [periodSize];
	while ((m_audioOutput -> bytesFree () >= periodSize) &&
	       (tempBuffer. GetRingBufferReadAvailable () >= periodSize)) {
	   tempBuffer. getDataFromBuffer (buffer, periodSize);
	   theWorker	-> write (buffer, periodSize);
	}
}

void	Qt_Audio::stop () {
	if (m_audioOutput != nullptr)
	   m_audioOutput        -> stop ();
        working. store (false);
}

void	Qt_Audio::restart	() {
	fprintf (stderr, "Going to restart with %d\n", newDeviceIndex);
	if (newDeviceIndex < 0)
	   return;
	if (m_audioOutput != nullptr)
	   delete m_audioOutput;
	m_audioOutput	= new QAudioSink (outputDevices. at (newDeviceIndex), 
	                                                      m_settings);
	m_audioOutput -> setBufferSize (10 * 480 * sizeof (float));
	theWorker	= m_audioOutput	-> start ();
	if (m_audioOutput -> error () == QAudio::NoError) {
	   working. store (true);
	   fprintf (stderr, "Device reports: no error\n");
	}
	else
	   fprintf (stderr, "restart gaat niet door\n");
	int vol		= audioSettings -> value (QT_AUDIO_VOLUME, 50). toInt ();
//	deviceList. at (newDeviceIndex). setVolume ((float)vol / 100);
	fprintf (stderr, "restarted\n");
	working. store (true);
}

bool	Qt_Audio::selectDevice	(int16_t index, const QString &s) {
	(void)s;
	newDeviceIndex	= index;
	stop ();
	restart ();
	return working. load ();
}

void	Qt_Audio::suspend	() {
	if (!working. load ())
	   return;
	m_audioOutput	-> suspend ();
}

void	Qt_Audio::resume	() {
	if (!working. load ())
	   return;
	m_audioOutput	-> resume ();
}

void	Qt_Audio::setVolume	(int v) {
	if (!working. load ())
           return;
	store (audioSettings, SOUND_HANDLING, QT_AUDIO_VOLUME, v);
        m_audioOutput -> setVolume ((float)v / 100);
}

#else
	Qt_Audio::Qt_Audio (QSettings *settings):
	                 tempBuffer (8 * 32768) { 
	audioSettings		= settings;
	outputRate		= 48000;	// default
	working. store		(false);
	isInitialized. store	(false);
	newDeviceIndex		= -1;
//
//	allDevices		= new QMediaDevices (nullptr);
//	deviceList	 	= allDevices -> audioOutputs ();
//	for (auto &deviceInfo : deviceList)
//	      theList. push_back (deviceInfo. description (),
//	                          QVariant:: fromValue (deviceInfo));
	initialize_deviceList	();
	initializeAudio (QAudioDeviceInfo::defaultOutputDevice());
}

void	Qt_Audio::initialize_deviceList () {	
	QAudioFormat audioFormat;
	audioFormat. setSampleRate      (outputRate);
        audioFormat. setChannelCount    (2);
        audioFormat. setSampleSize      (sizeof (float) * 8);
        audioFormat. setCodec           ("audio/pcm");
        audioFormat. setByteOrder       (QAudioFormat::LittleEndian);
        audioFormat. setSampleType      (QAudioFormat::Float);

	const QAudioDeviceInfo &defaultDeviceInfo =
	                QAudioDeviceInfo::defaultOutputDevice ();
	if (!defaultDeviceInfo. isFormatSupported (audioFormat))
	   fprintf (stderr, "Default device does not support 48000\n");
	else {
	   fprintf (stderr, "Default device does support 48000\n");
	   theList. push_back (defaultDeviceInfo);
	}
	for (auto &deviceInfo:
	       QAudioDeviceInfo::availableDevices (QAudio::AudioOutput)) {
	   if (deviceInfo != defaultDeviceInfo) {
	      if (deviceInfo. isFormatSupported (audioFormat))
	         theList. push_back (deviceInfo);
	   }
	}

//	fprintf (stderr, "The devicelist \n");
//	for (auto & listEl: theList)
//	   fprintf (stderr, "%s\n",  listEl. deviceName (). toLatin1 (). data ());;

//	fprintf (stderr, "Length of deviceList %d\n",  theList. size ());
	if (theList. size () == 0)
	   throw (22);
}

	Qt_Audio::~Qt_Audio () {
}

QStringList	Qt_Audio::streams	() {
QStringList nameList;
	for (auto & listEl: theList)
	   nameList << listEl. deviceName ();
	return nameList;
}
//
//	Note that - by convention - all audio samples here
//	are in a rate 48000
void	Qt_Audio::audioOutput (float *fragment, int32_t size) {
	if (!working. load ())
	   return;
	int aa = tempBuffer. GetRingBufferWriteAvailable ();
	aa	= std::min ((int)(size * sizeof (float)), aa);
	aa	&= ~03;
	tempBuffer. putDataIntoBuffer ((char *)fragment, aa);
	int periodSize = m_audioOutput -> periodSize ();
	char buffer [periodSize];
	while ((m_audioOutput -> bytesFree () >= periodSize) &&
	       (tempBuffer. GetRingBufferReadAvailable () >= periodSize)) {
	   tempBuffer. getDataFromBuffer (buffer, periodSize);
	   theWorker	-> write (buffer, periodSize);
	}
}

void	Qt_Audio::initializeAudio(const QAudioDeviceInfo &deviceInfo) {
	audioFormat. setSampleRate	(outputRate);
	audioFormat. setChannelCount	(2);
	audioFormat. setSampleSize	(sizeof (float) * 8);
	audioFormat. setCodec		("audio/pcm");
	audioFormat. setByteOrder	(QAudioFormat::LittleEndian);
	audioFormat. setSampleType	(QAudioFormat::Float);

	if (!deviceInfo. isFormatSupported (audioFormat)) {
           audioFormat = deviceInfo.nearestFormat (audioFormat);
	}
	isInitialized. store (false);
	if (deviceInfo. isFormatSupported (audioFormat)) {
	   m_audioOutput. reset (new QAudioOutput (audioFormat));
	   if (m_audioOutput -> error () == QAudio::NoError) {
	      isInitialized. store (true);
//	      fprintf (stderr, "Initialization went OK\n");
	   }
//	   else
//	     fprintf (stderr, "Audio device gives error\n");
	}
}

void	Qt_Audio::stop () {
	m_audioOutput	-> stop ();
	working. store (false);
	isInitialized. store (false);
}

void	Qt_Audio::restart	() {
//	fprintf (stderr, "Going to restart with %d\n", newDeviceIndex);
	if (newDeviceIndex < 0)
	   return;
	initializeAudio (theList. at (newDeviceIndex));
	if (!isInitialized. load ()) {
	   fprintf (stderr, "Init failed for device %d\n", newDeviceIndex);
	   return;
	}
	theWorker	= m_audioOutput	-> start ();
	if (m_audioOutput -> error () == QAudio::NoError) {
	   working. store (true);
//	   fprintf (stderr, "Device reports: no error\n");
	}
	else
	   fprintf (stderr, "restart gaat niet door\n");
	int vol		= value_i (audioSettings, SOUND_HANDLING,
	                                  QT_AUDIO_VOLUME, 50);
	m_audioOutput	-> setVolume ((float)vol / 100);
}

bool	Qt_Audio::selectDevice	(int16_t index, const QString &s) {
	(void)s;
	newDeviceIndex	= index;
	stop ();
	restart ();
	return working. load ();
}

void	Qt_Audio::suspend	() {
	if (!working. load ())
	   return;
	m_audioOutput	-> suspend ();
}

void	Qt_Audio::resume	() {
	if (!working. load ())
	   return;
	m_audioOutput	-> resume ();
}

void	Qt_Audio::setVolume	(int v) {
	store (audioSettings, SOUND_HANDLING, QT_AUDIO_VOLUME, v);
	m_audioOutput	-> setVolume ((float)v / 100);
}
#endif
