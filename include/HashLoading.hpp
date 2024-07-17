#ifndef INCLUDE_HASHLOADING_HPP_
#define INCLUDE_HASHLOADING_HPP_

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

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

The idea is to reduce the size of the - immense - hashes multimap.
It has to be a multimap because we are operating on the premise that similar
song hashes collide, hence a normal map would not be sufficient.
*/
inline auto load_song_hashes(const fs::path &hahses_csv) {
  std::ifstream input_file;
  input_file.open(hahses_csv);
  std::unordered_multimap<uint32_t, std::pair<size_t, size_t>> hashes;
  std::string line;
  while (std::getline(input_file, line)) {
    std::stringstream line_sstream(line);
    std::string item;

    uint32_t key = 0;
    size_t time = 0;
    size_t songid = 0;

    // Read and parse each line
    if (std::getline(line_sstream, item, ',')) {
      key = static_cast<uint32_t>(std::stoul(item));
    }
    if (std::getline(line_sstream, item, ',')) {
      time = std::stoul(item);
    }
    if (std::getline(line_sstream, item, ',')) {
      songid = std::stoul(item);
    }

    // Insert the parsed values into the unordered_multimap
    hashes.emplace(key, std::make_pair(time, songid));
  }
  return hashes;
}

inline auto load_song_ids(const fs::path &songs_csv)
    -> std::unordered_map<size_t, fs::path> {
  std::unordered_map<size_t, fs::path> wav_files;
  std::ifstream file(songs_csv);

  if (!file.is_open()) {
    // std::cerr << "Error opening file: " << songs_csv << std::endl;
    return wav_files;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream line_sstream(line);
    std::string key_str;
    std::string value;

    // Get the key part
    std::getline(line_sstream, key_str, ',');

    // Remove quotes from the value part
    std::getline(line_sstream, value, ',');
    value = value.substr(1, value.size() -
                                2); // Remove the leading and trailing quotes

    auto key = std::stoul(key_str); // Convert key to integer
    wav_files[key] = value;
  }

  return wav_files;
}

#endif // INCLUDE_HASHLOADING_HPP_
