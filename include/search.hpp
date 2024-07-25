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

#include <csvdumps.hpp>
#include <tuple>

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

/// @brief score the similarity of clip to song
/// @tparam T
/// @param hashes hashmap of hashes :v
/// @param filenames hashmap, translation table from id to songname
/// @param searchsong audio query
/// @return std::vector<pair<score,song_id>>
template <std::floating_point T>
auto score_songs(
    const std::unordered_multimap<uint32_t, std::pair<size_t, size_t>> &hashes,
    const std::unordered_map<size_t, fs::path> &filenames,
    const Audio<T> &searchsong, Config cfg)
    -> std::vector<std::pair<size_t, size_t>> {

  Spectrogram spec(searchsong, cfg);

  auto pts = spec.get_local_maximums();

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
      // note that time is anchor in song and hash.second is anchor in sample
      if (!matches.contains(songid)) {
        matches[songid] = std::vector<std::pair<size_t, size_t>>();
      }
      // push back the time in song and in the hash being searched
      matches[songid].emplace_back(time, hash.second);
    }
  }

  std::vector<std::pair<size_t, size_t>> scores_and_songs;
  scores_and_songs.reserve(matches.size());
  for (const auto &match : matches) {
    auto songid = match.first;
    /// hyperparameter binsize!
    size_t score = score_matches(match.second, BIN_SIZE);
    scores_and_songs.emplace_back(score, songid);
  }

  return scores_and_songs;
}

template <std::floating_point T>
auto search_song(
    const std::unordered_multimap<uint32_t, std::pair<size_t, size_t>> &hashes,
    const std::unordered_map<size_t, fs::path> &filenames,
    const Audio<T> &searchsong) -> std::optional<fs::path> {

  Spectrogram spec(searchsong);

  csv_write_spectrogram(spec, "experiments/dumps/sample_spec.csv");

  auto pts = spec.get_local_maximums();

  csv_write_local_maxima(pts, "experiments/dumps/sample_crits.csv");

  std::vector<std::pair<uint32_t, size_t>> to_search_hashes = spec.get_hashes();

  csv_write_hashes(to_search_hashes, "experiments/dumps/sample_hashes.csv");

  // we need 1 series per found song, like [<timesong,timesample>,...]
  std::unordered_map<size_t, std::vector<std::pair<size_t, size_t>>> matches;
  // for debugging purposes, record each matching hash series.
  std::unordered_map<size_t, std::vector<std::tuple<uint32_t, size_t, size_t>>>
      per_song_matching_hashes;
  for (const auto &hash : to_search_hashes) {

    // find matches for this hash
    auto range = hashes.equal_range(hash.first);

    // on each match, separate it by song into distinct series
    for (auto it = range.first; it != range.second; ++it) {
      size_t time = it->second.first;
      size_t songid = it->second.second;
      // note that time is anchor in song and hash.second is anchor in sample
      if (!matches.contains(songid)) {
        matches[songid] = std::vector<std::pair<size_t, size_t>>();
        per_song_matching_hashes[songid] =
            std::vector<std::tuple<uint32_t, size_t, size_t>>();
      }
      // push back the time in song and in the hash being searched
      matches[songid].emplace_back(time, hash.second);
      per_song_matching_hashes[songid].emplace_back(hash.first, time,
                                                    hash.second);
    }
  }

  std::vector<std::pair<size_t, size_t>> songs_and_scores;
  songs_and_scores.reserve(matches.size());

  size_t scores_sum = 0;

  // Calculate scores and accumulate the sum of scores
  for (const auto &match : matches) {
    auto songid = match.first;
    /// hyperparameter binsize!
    size_t score = score_matches(match.second, BIN_SIZE);
    songs_and_scores.emplace_back(score, songid);
    scores_sum += score;
  }

  double avg_score = static_cast<double>(scores_sum) /
                     static_cast<double>(!matches.empty() ? matches.size() : 1);
  const double MAX_ALLOWED_STDDEV = avg_score * 0.5;

  // Calculate standard deviation
  double sum_squared_diff = std::transform_reduce(
      songs_and_scores.begin(), songs_and_scores.end(), 0.0, std::plus<>(),
      [avg_score](const std::pair<size_t, size_t> &score) {
        return std::pow(static_cast<double>(score.first) - avg_score, 2);
      });

  double stddev = std::sqrt(sum_squared_diff /
                            (!songs_and_scores.empty()
                                 ? static_cast<double>(songs_and_scores.size())
                                 : 1));

  std::cout << "avg score: " << avg_score << '\n';
  std::cout << "stddev: " << stddev << '\n';

  // Create a new vector to store z-scores
  std::vector<std::pair<double, size_t>> z_scores;
  z_scores.reserve(songs_and_scores.size());

  // Calculate z-scores for each song
  for (auto &[score, song_id] : songs_and_scores) {
    double z_score =
        (static_cast<double>(score) - avg_score) / (stddev ? stddev : 1.0);
    z_scores.emplace_back(z_score, song_id);
  }

  // Sort the songs by z-score in descending order
  std::ranges::sort(songs_and_scores,
                    [](const std::pair<size_t, size_t> &score_1,
                       const std::pair<size_t, size_t> &score_2) {
                      return score_1.first > score_2.first;
                    });
  std::ranges::sort(z_scores, [](const std::pair<double, size_t> &score_1,
                                 const std::pair<double, size_t> &score_2) {
    return score_1.first > score_2.first;
  });

  // cout top 5 matches
  std::cout << "Top 5 matches with normal score: " << '\n';
  for (size_t i = 0;
       i < std::min(songs_and_scores.size(), static_cast<size_t>(5)); ++i) {
    std::cout << "Song: " << filenames.at(songs_and_scores[i].second)
              << " Score: " << songs_and_scores[i].first << '\n';
  }

  std::cout << "Top 5 matches with z-score: " << '\n';
  for (size_t i = 0; i < std::min(z_scores.size(), static_cast<size_t>(5));
       ++i) {
    std::cout << "Song: " << filenames.at(z_scores[i].second)
              << " Score: " << z_scores[i].first << '\n';
  }

    std::cout << "Matches found: " << songs_and_scores.size() << '\n';

  // no matches
  if (songs_and_scores.empty()) {
    return std::nullopt;
  }

  return filenames.at(songs_and_scores[0].second);
}

#endif // INCLUDE_SEARCH_HPP_
