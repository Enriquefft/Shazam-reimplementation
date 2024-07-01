#ifndef INCLUDE_SPECTROGRAM_HPP_
#define INCLUDE_SPECTROGRAM_HPP_

#include "AudioFile.hpp"
#include <utility>
#include <vector>

constexpr auto DEFAULT_NFFT = 2048;

enum class WINDOW_FUNCT { HANN };
enum class PADDING_MODE { CONSTANT };

template <std::floating_point T> class Spectrogram {
private:
  struct DataPoint {
    size_t hertz;
    size_t time;
    T intensity;
  };

  std::vector<std::vector<T>> m_spectrogram;
  std::vector<DataPoint> m_features;

  void stft(const Audio<T> &audio, const size_t &n_fft = DEFAULT_NFFT,
            const std::optional<size_t> &hop_length = std::nullopt,
            const std::optional<size_t> &window_length = std::nullopt,
            const WINDOW_FUNCT &window = WINDOW_FUNCT::HANN, bool center = true,
            const PADDING_MODE &padding_mode = PADDING_MODE::CONSTANT);
  auto empty_like(std::pair<size_t, size_t> dimensions)
      -> std::vector<std::vector<T>>;

  auto frame(const std::vector<T> &audiodata, size_t frame_length,
             size_t hop_length) -> std::vector<std::vector<T>>;
  auto get_window(const WINDOW_FUNCT &window, const size_t &n_points);
  static auto hann(size_t n_points) -> std::vector<T>;
  static auto
  pad_center(const std::vector<T> &data, const size_t &target_size,
             const PADDING_MODE &padding_mode = PADDING_MODE::CONSTANT)

      -> std::vector<T>;
  static auto pad(const std::vector<T> &data,
                  const std::pair<int64_t, int64_t> &pad_width,
                  const PADDING_MODE &padding_mode = PADDING_MODE::CONSTANT,
                  const T &constant_value = T{}) -> std::vector<T>;
  static auto expand_to(const std::vector<T> &data, const size_t &target_dim);

public:
  // TODO(Enrique, Claudia): generate the Spectrogram
  explicit Spectrogram(const Audio<T> &audio);

  auto get_spectrogram() -> std::vector<std::vector<T>>;

  // TODO(JuanDiego, Luise): Extract the features from the spectrogram
  auto get_local_maximums() -> std::vector<DataPoint>;
};

template <std::floating_point T>
explicit Spectrogram(const Audio<T> &audio) -> Spectrogram<T>;

extern template class Spectrogram<float>;
extern template class Spectrogram<double>;

#endif // INCLUDE_SPECTROGRAM_HPP_
