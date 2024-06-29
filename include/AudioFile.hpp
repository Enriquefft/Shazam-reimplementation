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
  // TODO(JuanDiego, Luise): Extract the features from the spectrogram
  auto get_local_maximums() -> std::vector<DataPoint>;
};

#endif // INCLUDE_AUDIOFILE_HPP_
