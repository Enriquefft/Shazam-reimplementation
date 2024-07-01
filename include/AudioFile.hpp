#ifndef INCLUDE_AUDIOFILE_HPP_
#define INCLUDE_AUDIOFILE_HPP_

#include <sndfile.h>
#include <string>
#include <vector>

// Temporal, to be revised
using HERTZ_T = int;
using TIME_T = int;
using INTENSITY_T = float;
using HASH_T = uint32_t;

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


  size_t getX();
  size_t getY();
  // TODO(JuanDiego, Luise): Extract the features from the spectrogram
  auto get_local_maximums() -> std::vector<DataPoint>;
  auto get_hashes() -> std::vector<uint32_t>;
private:

//================================= MAXIMA FINDING ========================================
  /// @brief Get local maxima by maxfiltering subtracting original and finding zeroes
  /// @param neigh size of sliding window
  /// @return local maxima points
  std::vector<DataPoint> maxima_MINLIST_algorithm(int neigh);
  /// @brief O(n) implementation of maxima_MINLIST_algorithm. May have slightly different results
  /// @param neigh size of sliding windowlocal maxima points
  /// @return local maxima points
  std::vector<DataPoint> maxima_MINLIST_algorithm_optimized(int neigh);
  /// @brief Find maximum points by first pulling candidates using optimized MINLIST and culling them with GTN
  /// @param maxfilter_s size of maxfilter window
  /// @param gtn_s size of GTN window
  /// @param thresh How much above average must a local maxima be.
  /// @return local maxima points
  std::vector<DataPoint> maxima_MINLISTGCN_algorithm(int maxfilter_s,int gtn_s,INTENSITY_T thresh);
  /// @brief get local maxima defined as all points that are greater that those of their neighborhood
  std::vector<DataPoint> maxima_GTN_algorithm(int neighbourhood,float thrsh);
// ================================= HASH GENERATION ========================================
};

#endif // INCLUDE_AUDIOFILE_HPP_
