#include "WeatherClient.h"
#include <curl/curl.h>
#include <stdexcept>
#include <string>
#include <iostream>

size_t WeatherClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t total = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), total);
    return total;
}

std::string WeatherClient::get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("curl_easy_init failed");
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "Request failed :"
                  << curl_easy_strerror(res) << "\n";
    }

    curl_easy_cleanup(curl);
    return response;
}



//okay perfect before i create the newfile just so I get a better understanding can you 
//explain to me line by line what the curl_test.cpp file is doing how each line is working 
//together just so I get a better understanding