#include <iostream>
#include "CLI11.hpp"
#include <curl/curl.h>
#include "WeatherClient.h"
#include "json.hpp"
//compile command
// g++ -std=c++17 -Wall -Wextra -pedantic -O2 -o app main.cpp WeatherClient.cpp -lcurl

using namespace std;

int main(int argc, char* argv[])
{
    CLI::App app{"Hiking Weather Project"};

    std::string location = "Mt Baldy";
    int start_time = 6;
    int duration = 1;


    app.add_option("--l, --location", location, "Location");
    app.add_option("--s, --start_time", start_time, "Start Time")->check(CLI::Range(0,23));
    app.add_option("--d, --duration", duration, "Duration of Hike")->check(CLI::PositiveNumber);
    CLI11_PARSE(app, argc, argv);

    // cout << "Location: " << location << endl;
    // cout << "Start Time: " << start_time << endl;
    // cout << "Duration of hike: " << duration << endl;

    //making location url safe with lambda function
    auto encode_for_query = [](const std::string& s) {
        string out;
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

    //make url to get lat and long
    const string encoded_location = encode_for_query(location);
    const string geo_url = "https://geocoding-api.open-meteo.com/v1/search?name="
                            + encoded_location + "&count=10&language=en&format=json";



    WeatherClient client;

    string geo_body = client.get(geo_url);
    using json = nlohmann::json;
    json j = json::parse(geo_body);

    if (!j.contains("results") || j["results"].empty()) {
        cerr << "No weather results for: " << location << "\n";
        return 1;
    }
    const json& first = j["results"][0];
    double lat = first.at("latitude").get<double>();
    double lon = first.at("longitude").get<double>();

    cout << "Location: " << location << "\n";
    cout << "Lat: " << lat << "\n";
    cout << "Lon: " << lon << "\n";

    //forecast api call
    const string forecast_url =
        "https://api.open-meteo.com/v1/forecast?latitude=" + std::to_string(lat) +
        "&longitude=" + std::to_string(lon) +
        "&current=temperature_2m,apparent_temperature,precipitation,weather_code,wind_speed_10m,wind_gusts_10m,wind_direction_10m"
        "&temperature_unit=fahrenheit"
        "&wind_speed_unit=mph"
        "&precipitation_unit=inch"
        "&timezone=auto";

    const string forecast_body = client.get(forecast_url);
    json j_forecast = json::parse(forecast_body);

    if (!j_forecast.contains("current") || j_forecast["current"].empty()) {
        cerr << "No Weather results for: " << location << "\n";
        return 1;
    }

    const json& current_conditions = j_forecast["current"];
    int weather_code = current_conditions.at("weather_code").get<int>();
    double temp = current_conditions.at("temperature_2m").get<double>();
    double apparent_temp = current_conditions.at("apparent_temperature").get<double>();
    double wind_speed = current_conditions.at("wind_speed_10m").get<double>();
    double precipitation = current_conditions.at("precipitation").get<double>();
    string curr_time = current_conditions.at("time").get<string>();

    cout << "Weather Code: " << weather_code << "\n";
    cout << "Current temp: " << temp << "\n";
    cout << "Apparent temp: " << apparent_temp << "\n";
    cout << "Wind Speed: " << wind_speed << "\n";
    cout << "Precipation: " << precipitation << "\n";
    cout << "Current time: " << curr_time << "\n";


}