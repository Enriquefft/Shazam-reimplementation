#include "AudioFile.hpp"
// #include "UnorderedMap.hpp"
#include <algorithm>

#include <deque>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <iostream>

#include <csvdumps.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <unordered_map>

using TypeParam = double;

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

  std::ifstream f;
  f.open(hahses_csv / "hashes.csv");
  std::unordered_multimap<uint32_t, std::pair<size_t, size_t>> hashes;
  std::string line;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    std::string item;
    uint32_t key = 0;
    size_t time = 0;
    size_t songid = 0;

    // Read and parse each line
    if (std::getline(ss, item, ',')) {
      key = static_cast<uint32_t>(std::stoul(item));
    }
    if (std::getline(ss, item, ',')) {
      time = static_cast<uint32_t>(std::stoul(item));
    }
    if (std::getline(ss, item, ',')) {
      songid = static_cast<uint32_t>(std::stoul(item));
    }

    // Insert the parsed values into the unordered_multimap
    hashes.emplace(key, std::make_pair(time, songid));
  }
  return hashes;
}

inline auto load_song_ids(const fs::path &songs_csv) {
  std::unordered_map<size_t, fs::path> wav_files;
  std::ifstream file(songs_csv / "songs.csv");

  if (!file.is_open()) {
    std::cerr << "Error opening file: " << songs_csv / "songs.csv" << '\n';
    return wav_files;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string key_str;
    std::string value;

    // Get the key part
    std::getline(ss, key_str, ',');

    // Remove quotes from the value part
    std::getline(ss, value, ',');
    value = value.substr(1, value.size() -
                                2); // Remove the leading and trailing quotes

    size_t key = std::stoul(key_str); // Convert key to integer
    wav_files[key] = value;
  }

  return wav_files;
}

// take 1 song and score it
inline auto score_matches(const std::vector<std::pair<size_t, size_t>> &matches,
                          size_t binsize) -> size_t {
  // get the timedeltas for the matches
  std::vector<size_t> deltas;
  for (const auto &match : matches) {

    auto [timeSong, timeQuery] = match;

    size_t delta =
        timeSong > timeQuery ? timeSong - timeQuery : timeQuery - timeSong;
    deltas.push_back(delta);
  }
  // sort in order of increasing value in order to build the hist.
  std::sort(deltas.begin(), deltas.end());

  // compute histogram peak+peakValue
  std::deque<size_t> bin;
  size_t max_bin_size = 0;

  for (const auto &delta : deltas) {
    bin.push_front(delta);

    // Calculate the lower limit as a int64_t
    int64_t lower_limit =
        static_cast<int64_t>(delta) - static_cast<int64_t>(binsize);

    // Convert bin.back() to int64_t for comparison
    auto signed_back = static_cast<int64_t>(bin.back());

    // Remove elements from the back of bin until the condition is satisfied
    while (!bin.empty() && signed_back < lower_limit) {
      bin.pop_back();
      if (!bin.empty()) {
        signed_back = static_cast<int64_t>(bin.back());
      }
    }

    // Update maxBinSize
    max_bin_size = std::max(max_bin_size, bin.size());
  }
  return max_bin_size;
}

inline auto search(
    const std::unordered_multimap<uint32_t, std::pair<size_t, size_t>> &hashes,
    const std::unordered_map<size_t, fs::path> &filenames,
    const fs::path &to_search) -> fs::path {
  Audio<TypeParam> searchsong(to_search);
  Spectrogram spec(searchsong);
  spec.get_local_maximums();
  auto to_search_hashes = spec.get_hashes();

  // we need 1 series per found song, like [<timesong,timesample>,...]
  std::unordered_map<size_t, std::vector<std::pair<size_t, size_t>>> matches;
  for (const auto &hash : to_search_hashes) {
    // find matches for this hash
    auto range = hashes.equal_range(hash.first);
    // on each match, separate it by song into distinct series
    for (auto it = range.first; it != range.second; ++it) {
      size_t time = it->second.first;
      size_t songid = it->second.second;
      if (!matches.contains(songid)) {
        matches[songid] = std::vector<std::pair<size_t, size_t>>();
      }
      // push back the time in song and in the hash being searched
      matches[songid].emplace_back(time, hash.second);
    }
  }

  // spdlog raw matches
  for (const auto &i : matches) {
    const fs::path &fname = filenames.at(i.first);
    size_t score = i.second.size();
    spdlog::debug("Song {} with {} matches.", fname.string(), score);
  }

  std::vector<std::pair<size_t, size_t>> songs_and_scores;
  for (const auto &match : matches) {
    auto songid = match.first;
    /// hyperparameter binsize!
    size_t score = score_matches(match.second, 10);
    songs_and_scores.emplace_back(score, songid);
  }

  std::sort(songs_and_scores.begin(), songs_and_scores.end(),
            [](std::pair<size_t, size_t> a, std::pair<size_t, size_t> b) {
              return a.first > b.first;
            });

  // SPDLOG SEARCH RESULT
  for (const auto &i : songs_and_scores) {
    const fs::path &fname = filenames.at(i.second);
    size_t score = i.first;
    spdlog::debug("Song {} with score {}.", fname.string(), score);
  }
  // handle the issue of no matches!
  fs::path fname;
  if (!songs_and_scores.empty()) {
    fname = filenames.at(songs_and_scores[0].second);
  } else {
    fname = ""; // no matches
  }
  return fname;
}

/// Parse arguments
inline auto parse_args(int argc, char *argv[])
    -> std::pair<fs::path, fs::path> {
  if (argc < 3) {
    spdlog::error("Usage: {} HASH_DIR SONG_FILE", argv[0]);
    throw std::runtime_error("Invalid arguments");
  }
  fs::path hash_dir = argv[1];
  fs::path song_file = argv[2];
  return std::make_pair(hash_dir, song_file);
}

auto main(int argc, char *argv[]) -> int {

  try {
    auto [hash_dir, song_file] = parse_args(argc, argv);

    spdlog::set_level(spdlog::level::debug);
    spdlog::stopwatch load_sw;
    auto hashes = load_song_hashes(hash_dir);
    auto songids = load_song_ids(hash_dir);
    spdlog::info("Done loading in {}", load_sw);

    spdlog::stopwatch search_sw;
    auto f = search(hashes, songids, song_file);
    spdlog::info("Done searching in {}", search_sw);
    std::cout << "Best match: " << f << '\n';

  } catch (const std::exception &e) {
    spdlog::error("Error: {}", e.what());
    return 1;
  }

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

                 std::string{SONG_TO_SEARCH.stem()},
    sw_total.elapsed().count());
   */
  return 0;
}
