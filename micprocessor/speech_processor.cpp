#include "speech_processor.hpp"

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

    curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("failed to init curl");

    // output verbose
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // set basic options
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpass__.c_str());
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    // craft custom header
    struct curl_slist * chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: audio/wav");
    chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
    //chunk = curl_slist_append(chunk, "Expect:");
    // add header
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    if (res != CURLE_OK)
        throw std::runtime_error("failed adding custom headers");

    // craft the FORM
    struct curl_httppost * formpost = NULL;
    struct curl_httppost * lastptr  = NULL;
    
    // add the file in the form
    curl_formadd(&formpost, 
                 &lastptr, 
                 CURLFORM_COPYNAME, "",
                 CURLFORM_FILE, filename.c_str(), 
                 CURLFORM_END);
    // add the form to the request
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    // set callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, speech_processor::callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
 
    // RUN
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        std::cerr << "error running speech_processor query: " 
                  <<  curl_easy_strerror(res) << std::endl;
    else {
        double speed_upload, total_time;
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
        fprintf(stderr, "Speed: %.3f bytes/sec during %.3f seconds\n",
                speed_upload, total_time);
    }
    curl_formfree(formpost);
    curl_slist_free_all(chunk);
    curl_easy_cleanup(curl);

    // remove the file from disk!
    std::remove(filename.c_str());
}

void speech_processor::callback(void * data, 
                                size_t size, 
                                size_t nmemb, 
                                void * object)
{
    static_cast<speech_processor*>(object)->reply(data, size, nmemb);
}

void speech_processor::reply(void * data, 
                             size_t size, 
                             size_t nmemb)
{
    std::string result;
    size_t realsize = size * nmemb;
    result.append((char*)data, realsize);
    // TODO: this is where we need to chain a callback
    std::cout << result << std::endl;
}
