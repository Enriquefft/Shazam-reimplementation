#include <ParseConfig.hpp>
#include <Spectrogram.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <stdexcept>

PEAK_FUNCTION stringToPeakFunction(const std::string& str) {
    if (str == "MINLIST") return PEAK_FUNCTION::MINLIST;
    if (str == "GTN") return PEAK_FUNCTION::GTN;
    if (str == "MINLISTGTN") return PEAK_FUNCTION::MINLISTGTN;
    throw std::invalid_argument("Invalid PEAK_FUNCTION value");
}

Config parseConfig(const std::string& filePath) {
    Config config;
    std::unordered_map<std::string, std::string> values;
    std::ifstream file(filePath);

    if (!file) {
        throw std::runtime_error("Could not open file");
    }

    std::string line;
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v")); // Trim leading whitespace
        if (line.empty() || line[0] == ';') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1); // Trim trailing whitespace
        value.erase(0, value.find_first_not_of(" \t\n\r\f\v")); // Trim leading whitespace

        values[key] = value;
    }

    try {
        if (values.find("FFT_WINDOW") != values.end()) config.FFT_WINDOW = std::stoi(values["FFT_WINDOW"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for FFT_WINDOW." << std::endl;
    }

    try {
        if (values.find("MINLIST_SIZEX") != values.end()) config.MINLIST_SIZEX = std::stoi(values["MINLIST_SIZEX"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for MINLIST_SIZEX." << std::endl;
    }

    try {
        if (values.find("MINLIST_SIZEY") != values.end()) config.MINLIST_SIZEY = std::stoi(values["MINLIST_SIZEY"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for MINLIST_SIZEY." << std::endl;
    }

    try {
        if (values.find("GTN_SIZE") != values.end()) config.GTN_SIZE = std::stoi(values["GTN_SIZE"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for GTN_SIZE." << std::endl;
    }

    try {
        if (values.find("GTN_THRESHOLD") != values.end()) config.GTN_THRESHOLD = std::stod(values["GTN_THRESHOLD"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for GTN_THRESHOLD." << std::endl;
    }

    try {
        if (values.find("PEAK_ALGORITHM") != values.end()) config.PEAK_ALGORITHM = stringToPeakFunction(values["PEAK_ALGORITHM"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for PEAK_ALGORITHM." << std::endl;
    }

    try {
        if (values.find("HASH_BOXX") != values.end()) config.HASH_BOXX = std::stoi(values["HASH_BOXX"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for HASH_BOXX." << std::endl;
    }

    try {
        if (values.find("HASH_BOXY") != values.end()) config.HASH_BOXY = std::stoi(values["HASH_BOXY"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for HASH_BOXY." << std::endl;
    }

    try {
        if (values.find("HASH_BOX_DISPLACEMENT") != values.end()) config.HASH_BOX_DISPLACEMENT = std::stoi(values["HASH_BOX_DISPLACEMENT"]);
    } catch (...) {
        std::cerr << "Warning: Using default value for HASH_BOX_DISPLACEMENT." << std::endl;
    }

    return config;
}