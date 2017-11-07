#ifndef CHATBOT_QUERY
#define CHATBOT_QUERY
#include "includes.ihh"
/**
 * @brief a wrapper class which POSTs chat queries to an HTTP mumbler
 * @version 0.1.0
 * @author Alex Giokas <a.gkiokas@ortelio.co.uk>
 */
struct chatbot_query
{
    static constexpr char URL[] = "10.130.3.2";
    AL::ALTextToSpeechProxy tts;

    /// ctor
    chatbot_query();

    /// @brief request a reply for the given input
    /// @warning @param filename must exist!
    void request(std::string input);

    /// libcurl callback function
    /// @param buffer is `filled` by libcurl
    /// @param size is the data received
    /// @param nmemb is ???
    /// @param object is a void pointer to `chatbot_query` *this
    static size_t callback(void * data, 
                           size_t size, 
                           size_t nmemb, 
                           void * object);

    /// @brief process the reply into an std::string
    size_t reply(void * data, 
                 size_t size, 
                 size_t nmemb);

};
#endif
