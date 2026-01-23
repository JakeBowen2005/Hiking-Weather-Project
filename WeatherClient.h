#pragma once
#include <string>

class WeatherClient {
public:
    std::string get(const std::string& url);

private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};