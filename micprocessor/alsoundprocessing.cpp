#include "alsoundprocessing.hpp"

microphone::microphone(boost::shared_ptr<ALBroker> pBroker,
                       std::string pName,
                       float noise_level)
: ALSoundExtractor(pBroker, pName), 
  threshold(noise_level)
{
    setModuleDescription("microphone monitor and data processor");
    // load IBM credentials from file
    std::ifstream t("ibm.credentials");
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    // allocate a S2T processor
    s2t = std::make_unique<speech_processor>(str);
    assert(s2t);
}

void microphone::init()
{
    fALMemoryKeys.push_back("microphone/leftMicEnergy");
    fALMemoryKeys.push_back("microphone/rightMicEnergy");
    fALMemoryKeys.push_back("microphone/frontMicEnergy");
    fALMemoryKeys.push_back("microphone/rearMicEnergy");
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
    qi::Application::atStop(boost::bind(&microphone::stopDetection, this));
#endif
    startDetection();
}

microphone::~microphone()
{
    stopDetection();
}

void microphone::process(const int & nbOfChannels,			// number of channels
                         const int & nbOfSamplesByChannel,	// amount of Samples per channel
                         const AL_SOUND_FORMAT * buffer,	// actual data buffer
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

void microphone::open_buffer(unsigned int channels,
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

void microphone::fill_buffer(unsigned int channels,
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

void microphone::close_buffer()
{
    // TODO: shouldn't we at this point, block the microphone ???
    assert(ptr);
    ptr->printSummary();
    std::time_t result = std::time(0);
    std::string filename = boost::lexical_cast<std::string>(result) + ".wav";
    ptr->save(filename);
    ptr.release();
    std::cout << "WAV: " << filename << std::endl;
    // callback speech_processor now
    s2t->request(filename);
}
