#include "chatbot_query.hpp"
#include "json/src/json.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
using json = nlohmann::json;

constexpr char chatbot_query::URL[];

chatbot_query::chatbot_query()
: tts("127.0.0.1", 9559)
{
    std::cout << "MUMBLER endpoint: " << URL << std::endl;
}

void chatbot_query::request(std::string query)
{
    CURL * curl;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("failed to init curl");
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // set basic options
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_PORT, 5000L);

    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    // append file to POST    
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, query.size());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "nao_mumbler");
    // set callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, chatbot_query::callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    // craft custom header
    struct curl_slist * chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
    // add header
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    if (res != CURLE_OK)
        throw std::runtime_error("failed adding custom headers");
    // RUN
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "error running chatbot_query query: " 
                  <<  curl_easy_strerror(res) << std::endl;
        throw std::runtime_error("chatbot_query error");
    }
    curl_slist_free_all(chunk);
    curl_easy_cleanup(curl);
}

size_t chatbot_query::callback(void * data, 
                               size_t size, 
                               size_t nmemb, 
                               void * object)
{
    return static_cast<chatbot_query*>(object)->reply(data, size, nmemb);
}

size_t chatbot_query::reply(void * data, 
                            size_t size, 
                            size_t nmemb)
{
    std::string result;
    size_t realsize = size * nmemb;
    result.append((char*)data, realsize);

    std::istringstream iss(result);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};

    std::string response;
    for (const auto & str : tokens) {
        std::string prefix_u("$user");
        std::string prefix_r("$robot");
        if (!str.compare(0, prefix_u.size(), prefix_u))
            response += " ";
        else if (!str.compare(0, prefix_r.size(), prefix_r))
            response += "Bob ";
        else
            response += str + " ";
    }
    std::cout << "[T2S] " << response << std::endl;
    tts.say(response);
    return realsize;
}
