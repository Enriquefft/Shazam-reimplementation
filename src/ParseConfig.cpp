#include <ParseConfig.hpp>
#include <Spectrogram.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

PEAK_FUNCTION parsePeakFunction(const std::string& value) {
    if (value == "MINLIST") return PEAK_FUNCTION::MINLIST;
    if (value == "GTN") return PEAK_FUNCTION::GTN;
    if (value == "MINLISTGTN") return PEAK_FUNCTION::MINLISTGTN;
    throw std::invalid_argument("Invalid value for PEAK_FUNCTION");
}

/// @brief parse the configuration from a file.
/// @param filepath path of the config.ini file
/// @return configuration object as per the config file
Config parseConfig(const std::string& filepath) {
    Config config;
    std::map<std::string, std::string> configMap;

    std::ifstream file(filepath);
    if (!file) {
        std::cerr << "Unable to open config file: " << filepath << std::endl;
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';') continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        // Remove spaces from key and value
        key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
        value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

        configMap[key] = value;
    }

    if (configMap.find("FFT_WINDOW") != configMap.end()) {
        config.FFT_WINDOW = std::stoi(configMap["FFT_WINDOW"]);
    }
    if (configMap.find("MINLIST_SIZEX") != configMap.end()) {
        config.MINLIST_SIZEX = std::stoi(configMap["MINLIST_SIZEX"]);
    }
    if (configMap.find("MINLIST_SIZEY") != configMap.end()) {
        config.MINLIST_SIZEY = std::stoi(configMap["MINLIST_SIZEY"]);
    }
    if (configMap.find("GTN_SIZE") != configMap.end()) {
        config.GTN_SIZE = std::stoi(configMap["GTN_SIZE"]);
    }
    if (configMap.find("GTN_THRESHOLD") != configMap.end()) {
        config.GTN_THRESHOLD = std::stod(configMap["GTN_THRESHOLD"]);
    }
    if (configMap.find("PEAK_ALGORITHM") != configMap.end()) {
        config.PEAK_ALGORITHM = parsePeakFunction(configMap["PEAK_ALGORITHM"]);
    }
    if (configMap.find("HASH_BOXX") != configMap.end()) {
        config.HASH_BOXX = std::stoi(configMap["HASH_BOXX"]);
    }
    if (configMap.find("HASH_BOXY") != configMap.end()) {
        config.HASH_BOXY = std::stoi(configMap["HASH_BOXY"]);
    }
    if (configMap.find("HASH_BOX_DISPLACEMENT") != configMap.end()) {
        config.HASH_BOX_DISPLACEMENT = std::stoi(configMap["HASH_BOX_DISPLACEMENT"]);
    }

    return config;
}