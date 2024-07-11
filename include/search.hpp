#ifndef INCLUDE_SEARCH_HPP_
#define INCLUDE_SEARCH_HPP_

#include <vector>
#include <deque>
#include <algorithm>
#include <AudioFile.hpp>
#include <Spectrogram.hpp>
#include <unordered_map>
#include <filesystem>
#include <concepts>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

namespace fs = std::filesystem;

// take 1 song and score it
size_t scoreMatches(const std::vector<std::pair<size_t,size_t>>& matches, size_t binsize)
{
  // get the timedeltas for the matches
  std::vector<size_t> deltas;
  for (const auto& i: matches)
  {
    size_t timeSong, timeQuery;
    timeSong = i.first;
    timeQuery = i.second;
    size_t delta = timeSong > timeQuery? 
            timeSong - timeQuery:timeQuery - timeSong;
    deltas.push_back(delta);
  }
  // sort in order of increasing value in order to build the hist.
  std::sort(deltas.begin(),deltas.end());

  // compute histogram peak+peakValue
  std::deque<size_t> bin;
  size_t maxBinSize = 0;

  for (const auto& i : deltas) {
      bin.push_front(i);

      // Calculate the lower limit as a long long int
      long long int lowerLimit = static_cast<long long int>(i) - static_cast<long long int>(binsize);

      // Convert bin.back() to long long int for comparison
      long long int signedBack = static_cast<long long int>(bin.back());

      // Remove elements from the back of bin until the condition is satisfied
      while (!bin.empty() && signedBack < lowerLimit) {
          bin.pop_back();
          if (!bin.empty()) {
              signedBack = static_cast<long long int>(bin.back());
          }
      }

      // Update maxBinSize
      maxBinSize = std::max(maxBinSize, bin.size());
    }
  return maxBinSize;
}

template<std::floating_point T>
fs::path search_song(const std::unordered_multimap<uint32_t,std::pair<size_t,size_t>>& hashes,
                const std::unordered_map<size_t, fs::path>& filenames,
                const Audio<T>& searchsong)
{
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

  // spdlog raw matches
  for (const auto& i:matches)
  {
     fs::path fname = filenames.at(i.first);
     size_t score = i.second.size();
     spdlog::debug("Song {} wih {} matches.",fname.string(),score);
  }


  std::vector<std::pair<size_t,size_t>> songs_and_scores;
  for (const auto& match:matches)
  {
    auto songid = match.first;
    /// hyperparameter binsize!
    size_t score = scoreMatches(match.second,10);
    songs_and_scores.push_back(std::make_pair(score,songid));
  }

  std::sort(songs_and_scores.begin(),songs_and_scores.end(),
  [](std::pair<size_t,size_t> a, std::pair<size_t,size_t> b)
  {
    return a.first > b.first;
  });

  for (const auto& i:songs_and_scores)
  {
     fs::path fname = filenames.at(i.second);
     size_t score = i.first;
     spdlog::debug("Song {} wih score {}",fname.string(),score);
  }
  
  // handle the issue of no matches!
  fs::path fname;
  if (songs_and_scores.size() > 0) 
    fname = filenames.at(songs_and_scores[0].second);
  else 
    fname = ""; // no matches
  return fname;
}

#endif