#ifndef INCLUDE_HASHLOADING_HPP_
#define INCLUDE_HASHLOADING_HPP_

#include <filesystem>
#include <unordered_map>
#include <sstream>
#include <fstream>


namespace fs = std::filesystem;
using hash_t = std::pair<uint32_t, size_t>;


// Functions to load the csvs. They are in the format:
/*
hashes:
<hash>,<pivot time>,<songid>
...
songs:
<songid>,<song file name>
...

The idea is to reduce the size of the - inmense - hashes multimap.
It has to be a multimap because we are operating on the premise that similar song hashes
collide, hence a normal map would not be sufficient.
*/
auto load_song_hashes(fs::path hahses_csv)
{
  std::ifstream f;
  f.open(hahses_csv);
  std::unordered_multimap<uint32_t,std::pair<size_t,size_t>> hashes;
  std::string line;
  while (std::getline(f, line)) {
      std::stringstream ss(line);
      std::string item;
      uint32_t key;
      size_t time, songid;

      // Read and parse each line
      if (std::getline(ss, item, ',')) {
          key = std::stoi(item);
      }
      if (std::getline(ss, item, ',')) {
          time = std::stoi(item);
      }
      if (std::getline(ss, item, ',')) {
          songid = std::stoi(item);
      }

      // Insert the parsed values into the unordered_multimap
      hashes.emplace(key, std::make_pair(time, songid));
  }
  return hashes;
}

auto load_song_ids(fs::path songs_csv) {
    std::unordered_map<size_t, fs::path> wavFiles;
    std::ifstream file(songs_csv);
    
    if (!file.is_open()) {
        // std::cerr << "Error opening file: " << songs_csv << std::endl;
        return wavFiles;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string keyStr, value;
        
        // Get the key part
        std::getline(ss, keyStr, ',');
        
        // Remove quotes from the value part
        std::getline(ss, value, ',');
        value = value.substr(1, value.size() - 2); // Remove the leading and trailing quotes
        
        int key = std::stoi(keyStr); // Convert key to integer
        wavFiles[key] = value;
    }
    
    return wavFiles;
}

#endif