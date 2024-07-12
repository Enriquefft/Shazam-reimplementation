#ifndef INCLUDE_SEARCH_HPP_
#define INCLUDE_SEARCH_HPP_

#include <AudioFile.hpp>
#include <Spectrogram.hpp>
#include <algorithm>
#include <deque>
#include <filesystem>
#include <functional>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

constexpr size_t BIN_SIZE = 10;
constexpr double MIN_ALLOWED_VARIANCE = 10;

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
  std::ranges::sort(deltas);

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
    const Audio<T> &searchsong) -> std::optional<fs::path> {

  Spectrogram spec(searchsong);

  spec.get_local_maximums();

  std::vector<std::pair<uint32_t, size_t>> to_search_hashes = spec.get_hashes();

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

  size_t scores_sum = 0;

  std::vector<std::pair<size_t, size_t>> songs_and_scores;
  songs_and_scores.reserve(matches.size());
  for (const auto &match : matches) {
    auto songid = match.first;
    /// hyperparameter binsize!
    size_t score = score_matches(match.second, BIN_SIZE);
    songs_and_scores.emplace_back(score, songid);
    scores_sum += score;
  }

  size_t avg_score = scores_sum / (!matches.empty() ? matches.size() : 1);

  double variance = std::transform_reduce(
      songs_and_scores.begin(), songs_and_scores.end(), 0.0, std::plus<>(),
      [avg_score](const std::pair<size_t, size_t> &score) {
        return std::pow(score.first - avg_score, 2);
      });
  variance /=
      (!songs_and_scores.empty() ? static_cast<double>(songs_and_scores.size())
                                 : 1);

  std::ranges::sort(songs_and_scores, [](std::pair<size_t, size_t> score_1,
                                         std::pair<size_t, size_t> score_2) {
    return score_1.first > score_2.first;
  });

  std::cout << "Variance: " << variance << '\n';
  for (const auto &[score, song_id] : songs_and_scores) {
    std::cout << "Song: " << filenames.at(song_id) << " Score: " << score
              << '\n';
  }

  // no matches
  if (songs_and_scores.empty() /* || variance < MAX_ALLOWED_VARIANCE */) {
    return std::nullopt;
  }

  return filenames.at(songs_and_scores[0].second);
}

#endif // INCLUDE_SEARCH_HPP_
