#ifndef SPEECH_PROCESSOR
#define SPEECH_PROCESSOR
#include "includes.ihh"
/**
 * @brief a wrapper class which loads a WAV file and POSTS it to IBM Watson
 * @note  IBM credentials must be correct!
 * @note the IBM URL is: "https://stream.watsonplatform.net/speech-to-text/api/v1/recognize"
 * @note the HTTP protocol uses: user:password for authentication
 * @note the HTTP header uses: "Content-Type: audio/wav"
 *                             "Transfer-Encoding: compress"
 */
class speech_processor
: private mumbler::agent
{
public:
    static constexpr char URL[] = 
    "https://stream.watsonplatform.net/speech-to-text/api/v1/recognize";

    /// @brief construct with a valid IBM username and password
    speech_processor(std::string credentials);

    /// @brief request speech recognition
    /// @warning @param filename must exist!
    void request(std::string filename);

    /// libcurl callback function
    /// @param buffer is `filled` by libcurl
    /// @param size is the data received
    /// @param nmemb is ???
    /// @param object is a void pointer to `speech_processor` *this
    static size_t callback(void * data, 
                           size_t size, 
                           size_t nmemb, 
                           void * object);

    /// @brief process the reply into an std::string
    size_t reply(void * data, 
                 size_t size, 
                 size_t nmemb);
    
private:
    std::ofstream ofs__;
    mumbler::reply_pool<std::string> pool__;
    std::string userpass__;
    std::string robotname__("nao");
    std::string username__("");
};

#endif
