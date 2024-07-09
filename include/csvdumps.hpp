#ifndef INCLUDE_CSVDUMPS_HPP_
#define INCLUDE_CSVDUMPS_HPP_

#include "Spectrogram.hpp"
#include <cstring>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

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
#endif // INCLUDE_CSVDUMPS_HPP_
