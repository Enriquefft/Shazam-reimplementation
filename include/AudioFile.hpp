#ifndef INCLUDE_AUDIOFILE_HPP_
#define INCLUDE_AUDIOFILE_HPP_

#include <filesystem>
#include <optional>
#include <sndfile.h>
#include <vector>

template <std::floating_point T> class Audio {
public:
  std::vector<T> m_audiodata;
  int m_sample_rate;

  /// @brief create an Audio class instance from a wav_file
  explicit Audio(const std::filesystem::path &path,
                 const std::optional<float> &sample_rate = std::nullopt);
};

template <std::floating_point T> class Spectrogram {
private:
  struct DataPoint {
    size_t hertz;
    size_t time;
    T intensity;
  };

  std::vector<std::vector<T>> m_spectrogram;
  std::vector<DataPoint> m_features;

  auto stft(const Audio<T> &audio, const auto &n_fft, const auto &hop_length,
            const auto &window_length);

public:
  // TODO(Enrique, Claudia): generate the Spectrogram
  explicit Spectrogram(const Audio<T> &audio);

  auto get_spectrogram() -> std::vector<std::vector<T>>;

  // TODO(JuanDiego, Luise): Extract the features from the spectrogram
  auto get_local_maximums() -> std::vector<DataPoint>;
};

template <std::floating_point T>
explicit Spectrogram(const Audio<T> &audio) -> Spectrogram<T>;

extern template class Audio<float>;
extern template class Audio<double>;

extern template class Spectrogram<float>;
extern template class Spectrogram<double>;

#endif // INCLUDE_AUDIOFILE_HPP_
