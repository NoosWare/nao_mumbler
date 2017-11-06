#include "speech_processor.hpp"
#include "json/src/json.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
using json = nlohmann::json;

constexpr char speech_processor::URL[];

speech_processor::speech_processor(std::string credentials)
: userpass__(credentials)
{
    std::cout << "IBM endpoint: " << URL << std::endl;
    std::cout << "IBM credentials: " << userpass__ << std::endl;
}

void speech_processor::request(std::string filename)
{
    CURL * curl;
    CURLcode res;
    struct stat file_info;
    FILE * fd = fopen(filename.c_str(), "rb");
    if (!fd)
        throw std::runtime_error("failed to open WAV");
    if (fstat(fileno(fd), &file_info) != 0) 
        throw std::runtime_error("can't get WAV info");
    curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("failed to init curl");
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // set basic options
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpass__.c_str());
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    // append file to POST    
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, file_info.st_size);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.47.0");
    // set callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, speech_processor::callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    // craft custom header
    struct curl_slist * chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: audio/wav");
    chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
    // add header
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    if (res != CURLE_OK)
        throw std::runtime_error("failed adding custom headers");
    // RUN
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        std::cerr << "error running speech_processor query: " 
                  <<  curl_easy_strerror(res) << std::endl;
    curl_slist_free_all(chunk);
    curl_easy_cleanup(curl);
    fclose(fd);
    std::remove(filename.c_str());
}

size_t speech_processor::callback(void * data, 
                                  size_t size, 
                                  size_t nmemb, 
                                  void * object)
{
    return static_cast<speech_processor*>(object)->reply(data, size, nmemb);
}

size_t speech_processor::reply(void * data, 
                              size_t size, 
                              size_t nmemb)
{
    std::string result;
    size_t realsize = size * nmemb;
    result.append((char*)data, realsize);
    
    json result_json;
    try {    
        result_json = json::parse(result);
    }
    catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
    for (const auto result : result_json["results"]) {
    //    auto alternatives_obj = result["alternatives"];
        for (const auto alternative :  result["alternatives"]) {
            std::string sentence = alternative["transcript"].get<std::string>();
            std::cout << sentence;
            size_t position = sentence.find(robotname__);
            if (position < sentence.length() && position >= 0)
                sentence.replace(position, robotname__.length(), "$robot");
            std::istringstream iss(sentence);
            std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                            std::istream_iterator<std::string>{}};
            auto s_t = mumbler::state_trait(tokens);
            auto a_t = mumbler::agent::on_policy(s_t, pool__);
			std::string response;
			for (const auto & str : reply) {
				std::string prefix_u("$user");
				std::string prefix_r("$robot");
				if (!str.compare(0, prefix_u.size(), prefix_u))
					response += username__ + " ";
				else if (!str.compare(0, prefix_r.size(), prefix_r))
					response += robotname__ + " ";
				else
					response += str + " ";
			}
        }
        std::cout << response;
        tts__.say(response);
    }
    // NOTE: result is a JSON which needs to be parsed!
    //
    return realsize;
}
