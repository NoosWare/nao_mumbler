#include "alsoundprocessing.hpp"
#include <alcommon/alproxy.h>
#include <iostream>
#include <numeric>
#include <cmath>

ALSoundProcessing::ALSoundProcessing(boost::shared_ptr<ALBroker> pBroker,
                                     std::string pName,
                                     float noise_level)
  : ALSoundExtractor(pBroker, pName),
    threshold(noise_level)
{
  setModuleDescription("This module processes the data collected by the "
                       "microphones and output in the ALMemory the RMS power "
                       "of each of the four channels.");
}

void ALSoundProcessing::init()
{
    fALMemoryKeys.push_back("ALSoundProcessing/leftMicEnergy");
    fALMemoryKeys.push_back("ALSoundProcessing/rightMicEnergy");
    fALMemoryKeys.push_back("ALSoundProcessing/frontMicEnergy");
    fALMemoryKeys.push_back("ALSoundProcessing/rearMicEnergy");
    fProxyToALMemory.insertData(fALMemoryKeys[0], 0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[1], 0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[2], 0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[3], 0.0f);
    audioDevice->callVoid("setClientPreferences",
                        getName(),                //Name of this module
                        frequency,                //48000 Hz requested
                        (int)ALLCHANNELS,         //4 Channels requested
                        1                         //Deinterleaving requested
                        );
#ifdef SOUNDPROCESSING_IS_REMOTE
    qi::Application::atStop(boost::bind(&ALSoundProcessing::stopDetection, this));
#endif
    startDetection();
}

ALSoundProcessing::~ALSoundProcessing()
{
  stopDetection();
}

void ALSoundProcessing::process(const int & nbOfChannels,			// number of channels
                                const int & nbOfSamplesByChannel,	// amount of Samples per channel
                                const AL_SOUND_FORMAT * buffer,		// actual data buffer
                                const ALValue & timeStamp)			// timestamp?
{
    std::array<float, 4> fMicsEnergy = { 0.f, 0.f, 0.f, 0.f};
	for(int channelInd = 0; channelInd < nbOfChannels; channelInd++){
		for(int i = 0 ; i < nbOfSamplesByChannel ; i++) {
      		fMicsEnergy[channelInd] += (float)buffer[nbOfSamplesByChannel * channelInd + i]
                                 	 * (float)buffer[nbOfSamplesByChannel * channelInd + i];
    	}
    	fMicsEnergy[channelInd] /= (float)nbOfSamplesByChannel;
    	fMicsEnergy[channelInd]  = sqrtf(fMicsEnergy[channelInd]);
    }
    if (count == window) {
        count = 0;
        level.fill(0.f);
    }
    float energy = fMicsEnergy[AL::FRONTCHANNEL];
    level[count] = energy; 
    float avg_mean = std::accumulate(level.begin(), level.begin() + count, 0.f);
    avg_mean /= (float)count;
    if (std::isnan(avg_mean)) {
        avg_mean = average;
    }
    float diff = level[count] - average;
    // diff positive and above threshold
    if ((diff - avg_mean) > threshold) {
        delay = 10;
        open_buffer(nbOfChannels, nbOfSamplesByChannel);
        fill_buffer(nbOfChannels, nbOfSamplesByChannel, buffer);
    }
    // diff zero or positive but small
    else {
        if (delay > 0) {
            fill_buffer(nbOfChannels, nbOfSamplesByChannel, buffer);
            delay--;
            if (delay == 0) {
                close_buffer();
            }
        }
        else {
            average = avg_mean;
        }
    }
    count++;
}

void ALSoundProcessing::open_buffer(unsigned int channels,
                                    unsigned int samples)
{
    if (!ptr) {
        ptr = std::make_unique<audio>();
        assert(ptr);
        ptr->setAudioBufferSize(channels, samples);
        ptr->setBitDepth(16);
        ptr->setSampleRate(frequency);
    }
}

void ALSoundProcessing::fill_buffer(unsigned int channels,
                                    unsigned int samples,
                                    const AL_SOUND_FORMAT * buffer)
{
    assert(ptr);
    for (int ch_idx = 0; ch_idx < channels; ch_idx++) {
		for (int sm_idx = 0 ; sm_idx < samples ; sm_idx++) {
      		ptr->samples[ch_idx].push_back(buffer[samples * ch_idx + sm_idx] / 32768.);
    	}
    }
}

void ALSoundProcessing::close_buffer()
{
    assert(ptr);
    ptr->printSummary();
    ptr->save("audio.wav");
    ptr.release();
}
