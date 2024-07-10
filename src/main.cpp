#include "AudioFile.hpp"
//#include "UnorderedMap.hpp"
#include <algorithm>

#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <csvdumps.hpp>
#include <unordered_map>

using TypeParam = double;

namespace fs = std::filesystem;
using hash_t = std::pair<uint32_t, size_t>;

// constexpr auto count_matches(const std::vector<hash_t> &song_hash,
//                              const std::vector<hash_t> &hashes) -> size_t {
//   size_t match_count = 0;

//   for (auto hash: song_hash)

//   return match_count;
// }


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
        std::cerr << "Error opening file: " << songs_csv << std::endl;
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

fs::path search(const std::unordered_multimap<uint32_t,std::pair<size_t,size_t>>& hashes,
                const std::unordered_map<size_t, fs::path>& filenames,
                const fs::path to_search)
{
  Audio<TypeParam> searchsong(to_search);
  Spectrogram spec(searchsong);
  spec.get_local_maximums();
  auto to_search_hashes = spec.get_hashes();

  // we need 1 series per found song, like [<timesong,timesample>,...]
  std::unordered_map<size_t,std::vector<std::pair<size_t,size_t>>> matches;
  for (const auto& hash:to_search_hashes)
  {
    // find matches for this hash
    auto range = hashes.equal_range(hash.first);
    // on each match, separate it by song into distinct series
    for (auto it = range.first; it != range.second; ++it)
    {
      size_t time = it->second.first;
      size_t songid = it->second.second;
      if (matches.count(songid) == 0) 
        matches[songid] = std::vector<std::pair<size_t,size_t>>();
      // push back the time in song and in the hash being searched
      matches[songid].push_back(std::make_pair(time,hash.second));
    }
  }
  for (auto& i:matches)
  {
    size_t songid = i.first;
    size_t matchcount = i.second.size();
    std::cout << filenames.at(songid) << ": " << matchcount << std::endl;
  }
  return "";
}

auto main() -> int {
  auto hashes = load_song_hashes("experiments/hashes/hashes.csv");
  auto songids = load_song_ids("experiments/hashes/songs.csv");
  search(hashes,songids,"assets/the_bidding_noisy_sample.wav");
  

/* 
  spdlog::set_level(spdlog::level::debug);
  spdlog::stopwatch sw_total; // Total time stopwatch

  const std::string HASHES_FILE = "songs_map.bin";
  const std::filesystem::path SONG_TO_SEARCH = "assets/banana_man_sample.wav";

  std::cout << "Searching for " << SONG_TO_SEARCH << " in the database\n";

  spdlog::stopwatch sw_step1;
  auto songs_map = read_songs_map_from_file<TypeParam>(HASHES_FILE);
  spdlog::debug("Step 1 (Read songs_map) took {} seconds",
                sw_step1.elapsed().count());

  spdlog::stopwatch sw_step2;
  Audio<TypeParam> song(SONG_TO_SEARCH);
  spdlog::debug("Step 2 generate song audiodata took {} seconds",
                sw_step2.elapsed().count());

  spdlog::stopwatch sw_step3;
  Spectrogram<TypeParam> spectrogram(song);
  spdlog::debug("Step 3 generate song spectrogram took {} seconds",
                sw_step3.elapsed().count());

  spdlog::stopwatch sw_step4;
  spectrogram.get_local_maximums();
  auto song_hash = spectrogram.get_hashes();
  spdlog::debug("Step 4 generate song hashes took {} seconds",
                sw_step4.elapsed().count());

  std::unordered_map<std::string, size_t> matches;

  spdlog::stopwatch sw_step5;
  for (const auto &[song_name, song_hashes] : songs_map) {
    matches[song_name] = count_matches(song_hash, song_hashes);
  }
  spdlog::debug(
      "Step 5 compare hash to every song in songs_map took {} seconds",
      sw_step5.elapsed().count());

  for (const auto &[song_name, count] : matches) {
    std::cout << song_name << " : " << count << '\n';
  }

  spdlog::info("Total time to search for {} in songs_index: {} ms",

               std::string{SONG_TO_SEARCH.stem()}, sw_total.elapsed().count());
 */
  return 0;
}
