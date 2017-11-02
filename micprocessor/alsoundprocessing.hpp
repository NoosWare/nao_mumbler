/**
* @author Gwennael Gate
* Copyright (c) Aldebaran Robotics 2010, 2011
*/


#ifndef SOUNDPROCESSING_H
#define SOUNDPROCESSING_H
#include <string>
#include <rttools/rttime.h>
#include <qi/application.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <alvalue/alvalue.h>
#include <alproxies/almemoryproxy.h>
#include <alaudio/alsoundextractor.h>
#include "AudioFile/AudioFile.h"

using namespace AL;

class ALSoundProcessing 
: public ALSoundExtractor
{
public:
    // CTOR
    ALSoundProcessing(boost::shared_ptr<ALBroker> pBroker, 
                      std::string pName,
                      float noise_level = 800.f);

    // DTOR
    virtual ~ALSoundProcessing();

    //method inherited from almodule that will be called after constructor
    void init();

    // @brief method automatically called by the AudioDevice module
    // @param nbOfChannels is number of channels (4)
    // @param nbrOfSamplesByChannel is number of samples per channel (?)
    // @param buffer is the actual buffer data (audio data per channel (?)
    // @param timeStamp is the timestamp
    //
    void process(const int & nbOfChannels,
               const int & nbrOfSamplesByChannel,
               const AL_SOUND_FORMAT * buffer,
               const ALValue & timeStamp);

private:
    
    // create an audio file buffer
    void open_buffer(unsigned int channels,
                     unsigned int samples);

    // fill the buffer
    void fill_buffer(unsigned int channels,
                     unsigned int samples,
                     const AL_SOUND_FORMAT * buffer);

    // close the audio file (using existing buffer)
    void close_buffer();

    ALMemoryProxy fProxyToALMemory;
    std::vector<std::string> fALMemoryKeys;

    unsigned int frequency = 48000;

    static constexpr unsigned int window = 60; // 600ms speech window
    float average = 0.f;                       // average microphone energy

    std::array<float,window> level{};          // energy level window
    float threshold = 800.f;                   // delta energy threshold
    unsigned int count  = 0;                   // speech window counter
    unsigned int delay  = 0;                   // delay closing file (allows for pauses)

    using audio  = AudioFile<double>;
    std::unique_ptr<audio> ptr;               // AudioFile pointer - valid only when recording
};
#endif
