#ifndef INCLUDE_CSVDUMPS_HPP_
#define INCLUDE_CSVDUMPS_HPP_

#include "Spectrogram.hpp"
#include <cstring>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <tuple>

template <std::floating_point T>
void csv_write_spectrogram(const Spectrogram<T> &spectrogram,
                           const std::string &file_to_write) {
  auto spec = spectrogram.get_spectrogram();
  // assume spectrogram is well defined, hence it has a 0th element
  size_t rows = spec.size();
  size_t cols = spec.at(0).size();

  std::ofstream output_file;
  output_file.open(file_to_write, std::fstream::out | std::fstream::trunc);

  // dump the spectrogram data to a csv
  output_file << rows << ',' << cols << '\n';
  for (size_t i = 0; i < rows; i++) {
    output_file << spec[i][0];
    for (size_t j = 1; j < cols; j++) {
      output_file << ',' << spec[i][j];
    }
    output_file << '\n';
  }

  output_file.close();
}

template <std::floating_point T>
void csv_write_spectrogram(const std::vector<std::vector<T>> &spectrogram,
                           const std::string &file_to_write) {
  // assume spectrogram is well defined, hence it has a 0th element
  size_t rows = spectrogram.size();
  size_t cols = spectrogram.at(0).size();

  std::ofstream output_file;
  output_file.open(file_to_write, std::fstream::out | std::fstream::trunc);

  // dump the spectrogram data to a csv
  output_file << rows << ',' << cols << '\n';
  for (size_t i = 0; i < rows; i++) {
    output_file << spectrogram[i][0];
    for (size_t j = 1; j < cols; j++) {
      output_file << ',' << spectrogram[i][j];
    }
    output_file << '\n';
  }

  output_file.close();
}

template <typename T>
void csv_write_local_maxima(const std::vector<T> &points,
                            const std::string &file_to_write) {

  std::ofstream output_file;
  output_file.open(file_to_write, std::fstream::out | std::fstream::trunc);

  for (const T &point : points) {
    output_file << point.time << ',' << point.hertz << ',' << point.intensity
                << '\n';
  }

  output_file.close();
}

inline void csv_write_hashes(std::vector<std::pair<uint32_t, size_t>> &hashes,
                             const std::string &file_to_write) {

  std::ofstream output_file;
  output_file.open(file_to_write, std::fstream::out | std::fstream::trunc);

  for (const auto &hash : hashes) {
    output_file << hash.first << ',' << hash.second << '\n';
  }

  output_file.close();
}



template <std::floating_point T>
inline void csv_write_matches_hashes(std::vector<std::tuple<uint32_t,size_t,size_t>> matches,
                             const std::string &file_to_write)
{
  std::ofstream output_file;
  output_file.open(file_to_write, std::fstream::out | std::fstream::trunc);

  for (auto i:matches)
    output_file << std::get<0>(i) << ',' << std::get<1>(i) 
                          << ',' << std::get<2>(i) << '\n';
  output_file.close();
}


void extract_hashes(uint32_t combined, uint16_t &hash_1, uint16_t &hash_2, uint16_t &delta_t) {
  constexpr uint16_t MASK = 0x3FF;
  
  hash_1 = (combined >> 20) & MASK;
  hash_2 = (combined >> 10) & MASK;
  delta_t = combined & MASK;
}


template <std::floating_point T>
inline void csv_write_matches_pts(std::vector<std::tuple<uint32_t,size_t,size_t>> matches,
                             const std::string &file_to_write)
{
  // each tuple is hash,time in song, time in sample;
  std::ofstream output_file;
  output_file.open(file_to_write, std::fstream::out | std::fstream::trunc);
  for (auto i:matches)
  {
      uint16_t pivot_freq;
      uint16_t other_freq;
      uint16_t deltat; 
      extract_hashes(std::get<0>(i),pivot_freq,other_freq,deltat);

      size_t pivotx = std::get<1>(i);
      size_t pivoty = pivot_freq;
      size_t otherx = std::get<1>(i) + deltat;
      size_t othery = other_freq;

      output_file << pivotx << ',' << pivoty << "\n";
      output_file << otherx << ',' << othery << "\n";
  } 
}
#endif // INCLUDE_CSVDUMPS_HPP_
