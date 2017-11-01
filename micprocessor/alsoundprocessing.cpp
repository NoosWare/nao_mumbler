#include "alsoundprocessing.hpp"
#include <alcommon/alproxy.h>
#include <iostream>

ALSoundProcessing::ALSoundProcessing(boost::shared_ptr<ALBroker> pBroker,
                                     std::string pName)
  : ALSoundExtractor(pBroker, pName)
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

    fProxyToALMemory.insertData(fALMemoryKeys[0],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[1],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[2],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[3],0.0f);

    // Do not call the function setClientPreferences in your constructor!
    // setClientPreferences : can be called after its construction!
    audioDevice->callVoid("setClientPreferences",
                        getName(),                //Name of this module
                        48000,                    //48000 Hz requested
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


/// Description: The name of this method should not be modified as this
/// method is automatically called by the AudioDevice Module.
void ALSoundProcessing::process(const int & nbOfChannels,			// number of channels
                                const int & nbOfSamplesByChannel,	// amount of Samples per channel
                                const AL_SOUND_FORMAT * buffer,		// actual data buffer
                                const ALValue & timeStamp)			// timestamp?
{
    /// Computation of the RMS power of the signal delivered by
    /// each microphone on a 170ms buffer
    /// init RMS power to 0
    //
    std::array<float, 4> fMicsEnergy = { 0.f, 0.f, 0.f, 0.f};

	/// Calculation of the RMS power
	for(int channelInd = 0; channelInd < nbOfChannels; channelInd++){
		for(int i = 0 ; i < nbOfSamplesByChannel ; i++) {
      		fMicsEnergy[channelInd] += (float)buffer[nbOfSamplesByChannel * channelInd + i]
                                 	 * (float)buffer[nbOfSamplesByChannel * channelInd + i];
    	}
    	fMicsEnergy[channelInd] /= (float)nbOfSamplesByChannel;
    	fMicsEnergy[channelInd]  = sqrtf(fMicsEnergy[channelInd]);
    }
    // reset the level array every 1000
    if (count == 1000) {
        count = 0;
        level.fill(0.f);
    }

    count++;
    // TODO: we can average energy from left, right, front if needed
    float e   = fMicsEnergy[2];
    level[count] = e;

    // TODO: calculate arithmetic mean and geometric mean using level ?

    std::cout << "Energy = " << e 
              << std::endl;

}
