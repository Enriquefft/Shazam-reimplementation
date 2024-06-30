#ifndef INCLUDE_AUDIOFILE_HPP_
#define INCLUDE_AUDIOFILE_HPP_

#include <sndfile.h>
#include <string>
#include <vector>

// Temporal, to be revised
using HERTZ_T = int;
using TIME_T = int;
using INTENSITY_T = float;

struct DataPoint {
  HERTZ_T hertz;
  TIME_T time;
  INTENSITY_T intensity;
};

class Audio {
public:
  std::vector<int16_t> m_samples;
  int m_rate;

  /// @brief create an Audio class instance from a wav_file
  explicit Audio(const std::string &path);
};

class Spectrogram {
private:
  std::vector<std::vector<INTENSITY_T>> m_spectrogram;

  std::vector<DataPoint> m_features;

public:
  // TODO(Enrique, Claudia): generate the Spectrogram
  explicit Spectrogram(const Audio &audio);
  explicit Spectrogram(std::string csvname);
  // TODO(JuanDiego, Luise): Extract the features from the spectrogram
  auto get_local_maximums() -> std::vector<DataPoint>;

private:
  /// @brief Get local maxima by maxfiltering subtracting original and finding zeroes
  /// @param neigh size of sliding window
  /// @return local maxima points
  std::vector<DataPoint> maxima_MINLIST_algorithm(int neigh);
  INTENSITY_T max_in_neigh(size_t maxX, size_t maxY, uint x, uint y, int n);
  /// @brief get local maxima defined as all points that are greater that those of their neighborhood
  std::vector<DataPoint> maxima_GTN_algorithm(int neighbourhood,float thrsh);
  /// @brief  utilkit
  /// @param maxX size of spectrogram x
  /// @param maxY size of spectrogram y
  /// @param x x of evaluated point
  /// @param y y of evaluated point
  /// @param n radius of neighbourhood
  /// @return this point is strictly greater than a square of halfide neighborhood centered on itself
  bool is_max_in_neigh(size_t maxX, size_t maxY, uint x, uint y, int n,float thrsh);
};

#endif // INCLUDE_AUDIOFILE_HPP_
