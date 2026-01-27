#include "Conditions.h"
#include <iostream>

void Conditions::print() const {
    std::cout << "\n== " << label << " ==\n";
    std::cout << "Time: " << time << "\n";
    std::cout << "Weather code: " << weather_code << "\n";
    std::cout << "Temp: " << temp_f << " F (feels like " << apparent_temp_f << " F)\n";
    std::cout << "Wind Speed: " << wind_speed_mph << "\n";
    std::cout << "Wind gusts: " << wind_gusts_mph << "\n";
    std::cout << "Precip: " << precipitation_in << " in\n";
}