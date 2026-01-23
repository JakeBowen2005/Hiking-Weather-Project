#pragma once
#include <string>

class Conditions {
public:
    std::string label;
    std::string time;
    int weather_code = 0;
    double temp_f = 0;
    double apparent_temp_f = 0;
    double wind_speed_mph = 0;
    double precipitation_in = 0;
    void print() const;

};