#include <iostream>
#include "CLI11.hpp"
#include <curl/curl.h>
#include "WeatherClient.h"
#include "json.hpp"
#include "Conditions.h"
//compile command
// g++ -std=c++17 -Wall -Wextra -pedantic -O2 \
-o app main.cpp WeatherClient.cpp Conditions.cpp -lcurl


int main(int argc, char* argv[])
{
    CLI::App app{"Hiking Weather Project"};

    std::string location = "Mt Baldy";
    int start_time = 6;
    int duration = 1;
    int increment = 1;


    app.add_option("--l, --location", location, "Location");
    app.add_option("--s, --start_time", start_time, "Start hour (0-23) for today")->check(CLI::Range(0,23));
    app.add_option("--i, --increment", increment, "Increment of hours for each forecast")->check(CLI::PositiveNumber);
    app.add_option("--d, --duration", duration, "Duration of the hike in hours")->check(CLI::PositiveNumber);
    CLI11_PARSE(app, argc, argv);

    // cout << "Location: " << location << endl;
    // cout << "Start Time: " << start_time << endl;
    // cout << "Duration of hike: " << duration << endl;

    //making location url safe with lambda function
    auto encode_for_query = [](const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for ( unsigned char c : s) {
            if (c == ' ') {
                out += "%20";
            } else {
                out.push_back(static_cast<char>(c));
            }
        }
        return out;
    };

    //function to get correct start time
    auto pad2 = [](int h) {
        std::string s = std::to_string(h);
        if (s.size() == 1) s= "0"+s;
        return s;
    };


    //make url to get lat and long
    const std::string encoded_location = encode_for_query(location);
    const std::string geo_url = "https://geocoding-api.open-meteo.com/v1/search?name="
                            + encoded_location + "&count=10&language=en&format=json";



    WeatherClient client;

    std::string geo_body = client.get(geo_url); //raw json text
    using json = nlohmann::json;
    json j = json::parse(geo_body);

    if (!j.contains("results") || j["results"].empty()) {
        std::cerr << "No weather results for: " << location << "\n";
        return 1;
    }
    const json& first = j["results"][0];
    double lat = first.at("latitude").get<double>();
    double lon = first.at("longitude").get<double>();

    std::cout << "Location: " << location << "\n";
    std::cout << "Lat: " << lat << "\n";
    std::cout << "Lon: " << lon << "\n";

    //forecast api call
    const std::string forecast_url =
        "https://api.open-meteo.com/v1/forecast?latitude=" + std::to_string(lat) +
        "&longitude=" + std::to_string(lon) +
        "&current=temperature_2m,apparent_temperature,precipitation,weather_code,wind_speed_10m,wind_gusts_10m,wind_direction_10m"
        "&temperature_unit=fahrenheit"
        "&wind_speed_unit=mph"
        "&precipitation_unit=inch"
        "&timezone=auto"
        "&hourly=temperature_2m,apparent_temperature,precipitation,weather_code,wind_speed_10m,wind_gusts_10m,wind_direction_10m"
        "&forecast_hour=48";

    // std::cout << "\n=== Forecast API URL ===\n";
    // std::cout << forecast_url << "\n\n";

    const std::string forecast_body = client.get(forecast_url); //raw json text
    // std::cout << "=== Raw Forecast API Response ===\n";
    // std::cout << forecast_body << "\n";

    json j_forecast = json::parse(forecast_body);

    if (!j_forecast.contains("current") || j_forecast["current"].empty()) {
        std::cerr << "No Weather results for: " << location << "\n";
        return 1;
    }

    // Current Conditions
    Conditions Curr;
    Curr.label = "Current Conditions of location";
    const json& current_conditions = j_forecast["current"];
    Curr.weather_code = current_conditions.at("weather_code").get<int>();
    Curr.temp_f = current_conditions.at("temperature_2m").get<double>();
    Curr.apparent_temp_f = current_conditions.at("apparent_temperature").get<double>();
    Curr.wind_speed_mph = current_conditions.at("wind_speed_10m").get<double>();
    Curr.precipitation_in = current_conditions.at("precipitation").get<double>();
    Curr.wind_gusts_mph = current_conditions.at("wind_gusts_10m").get<double>();
    Curr.time = current_conditions.at("time").get<std::string>();
    Curr.print();

    //Build timestamp for start time
    const std::string date_part = Curr.time.substr(0,10);
    const std::string start_target_time = date_part + "T" + pad2(start_time) + ":00";

    if (!j_forecast.contains("hourly") || !j_forecast["hourly"].contains("time")) {
        std::cerr << "Forecast response missing hourly.time; cannot look up start hour \n";
        return 1;
    }

    const json& hourly = j_forecast["hourly"];
    const auto& times = hourly.at("time");

    int start_idx = -1;
    for (std::size_t i = 0; i < times.size(); i++) {
        if (times[i].get<std::string>() == start_target_time) {
            start_idx = static_cast<int>(i);
            break;
        }
    }
    
    if (start_idx == -1) {
        std::cerr << "Could not find start time in hourly forecast";
        return 1;
    } 

    //Start time conditions
    Conditions Start;
    Start.label = "Starting Time of the hike";
    Start.time = times[start_idx].get<std::string>();
    Start.weather_code = hourly.at("weather_code")[start_idx].get<int>();
    Start.temp_f = hourly.at("temperature_2m")[start_idx].get<double>();
    Start.apparent_temp_f = hourly.at("apparent_temperature")[start_idx].get<double>();
    Start.wind_speed_mph = hourly.at("wind_speed_10m")[start_idx].get<double>();
    Start.wind_gusts_mph = hourly.at("wind_gusts_10m")[start_idx].get<double>();
    Start.precipitation_in = hourly.at("precipitation")[start_idx].get<double>();
    Start.print();

    // End time conditions (simple index math)
    int end_idx = start_idx + duration;

    if (end_idx >= static_cast<int>(times.size())) {
        std::cerr << "Hike extends beyond available forecast data.\n";
        return 1;
    }

    Conditions End;
    End.label = "End Time of the hike";
    End.time = times[end_idx].get<std::string>();
    End.weather_code = hourly.at("weather_code")[end_idx].get<int>();
    End.temp_f = hourly.at("temperature_2m")[end_idx].get<double>();
    End.apparent_temp_f = hourly.at("apparent_temperature")[end_idx].get<double>();
    End.wind_speed_mph = hourly.at("wind_speed_10m")[end_idx].get<double>();
    End.wind_gusts_mph = hourly.at("wind_gusts_10m")[end_idx].get<double>();
    End.precipitation_in = hourly.at("precipitation")[end_idx].get<double>();
    End.print();
}