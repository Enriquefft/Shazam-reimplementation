#ifndef INCLUDE_SEARCH_HPP_
#define INCLUDE_SEARCH_HPP_

#include <AudioFile.hpp>
#include <Spectrogram.hpp>
#include <algorithm>
#include <deque>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

// take 1 song and score it
inline auto score_matches(const std::vector<std::pair<size_t, size_t>> &matches,
                          size_t binsize) -> size_t {
  // get the timedeltas for the matches
  std::vector<size_t> deltas;
  for (const auto &match : matches) {

    auto [time_song, time_query] = match;

    size_t delta = time_song > time_query ? time_song - time_query
                                          : time_query - time_song;
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

template <std::floating_point T>
auto search_song(
    const std::unordered_multimap<uint32_t, std::pair<size_t, size_t>> &hashes,
    const std::unordered_map<size_t, fs::path> &filenames,
    const Audio<T> &searchsong) -> fs::path {
  spdlog::info("Searching for song...");
  Spectrogram spec(searchsong);

  std::cout << "Generated spectrogram" << '\n';

  spec.get_local_maximums();

  std::cout << "Got local maximums" << '\n';

  auto to_search_hashes = spec.get_hashes();

  std::cout << "Got hashes" << '\n';

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
      matches[songid].push_back(std::make_pair(time, hash.second));
    }
  }

  // spdlog raw matches
  for (const auto &match : matches) {
    const fs::path &fname = filenames.at(match.first);
    size_t score = match.second.size();
    spdlog::debug("Song {} with {} matches.", fname.string(), score);
  }

  std::vector<std::pair<size_t, size_t>> songs_and_scores;
  for (const auto &match : matches) {
    auto songid = match.first;
    /// hyperparameter binsize!
    size_t score = score_matches(match.second, 10);
    songs_and_scores.emplace_back(score, songid);
  }

  std::sort(
      songs_and_scores.begin(), songs_and_scores.end(),
      [](std::pair<size_t, size_t> score_1, std::pair<size_t, size_t> score_2) {
        return score_1.first > score_2.first;
      });

  for (const auto &song_info : songs_and_scores) {
    const fs::path &fname = filenames.at(song_info.second);
    size_t score = song_info.first;
    spdlog::debug("Song {} with score {}", fname.string(), score);
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

#endif // INCLUDE_SEARCH_HPP_
