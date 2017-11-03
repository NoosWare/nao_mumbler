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
{
public:
    static constexpr char URL[] = 
    "10.130.3.26:4444";
    //"https://stream.watsonplatform.net/speech-to-text/api/v1/recognize";

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
    static void callback(void * data, 
                         size_t size, 
                         size_t nmemb, 
                         void * object);

    /// @brief process the reply into an std::string
    void reply(void * data, 
               size_t size, 
               size_t nmemb);
    
private:
    std::string userpass__;
};

#endif