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

using namespace AL;

class ALSoundProcessing 
: public ALSoundExtractor
{
public:
    // CTOR
    ALSoundProcessing(boost::shared_ptr<ALBroker> pBroker, 
                      std::string pName);

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
    ALMemoryProxy fProxyToALMemory;
    std::vector<std::string> fALMemoryKeys;
    unsigned int count = 0;
    float geo_mu = 0;
    float avg_mu = 0; 
    std::array<float,1000> level{};

    // TODO: we need a temporary WAV object (create on detection of voice,
    //                                       save on end of voice)
    //
    // TODO: we need a noise filter:
    //
    // F = 10 * log10 ( Geometric mean (E) / Arithmetic mean (E) )
    //
};
#endif
