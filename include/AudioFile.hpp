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
  explicit Spectrogram(const Audio &audio);
  explicit Spectrogram(std::string csvname);

  size_t getX();
  size_t getY();
  explicit Spectrogram(const Audio<T> &audio);

  auto get_spectrogram() -> std::vector<std::vector<T>>;

  // TODO(JuanDiego, Luise): Extract the features from the spectrogram
  auto get_local_maximums() -> std::vector<DataPoint>;
  auto get_hashes() -> std::vector<uint32_t>;

private:
  //================================= MAXIMA FINDING
  //========================================
  /// @brief Get local maxima by maxfiltering subtracting original and finding
  /// zeroes
  /// @param neigh size of sliding window
  /// @return local maxima points
  std::vector<DataPoint> maxima_MINLIST_algorithm(int neigh);
  /// @brief O(n) implementation of maxima_MINLIST_algorithm. May have slightly
  /// different results
  /// @param neigh size of sliding windowlocal maxima points
  /// @return local maxima points
  std::vector<DataPoint> maxima_MINLIST_algorithm_optimized(int neigh);
  /// @brief Find maximum points by first pulling candidates using optimized
  /// MINLIST and culling them with GTN
  /// @param maxfilter_s size of maxfilter window
  /// @param gtn_s size of GTN window
  /// @param thresh How much above average must a local maxima be.
  /// @return local maxima points
  std::vector<DataPoint> maxima_MINLISTGCN_algorithm(int maxfilter_s, int gtn_s,
                                                     INTENSITY_T thresh);
  /// @brief get local maxima defined as all points that are greater that those
  /// of their neighborhood
  std::vector<DataPoint> maxima_GTN_algorithm(int neighbourhood, float thrsh);
  // ================================= HASH GENERATION
  // ========================================
};

template <std::floating_point T>
explicit Spectrogram(const Audio<T> &audio) -> Spectrogram<T>;

extern template class Audio<float>;
extern template class Audio<double>;

extern template class Spectrogram<float>;
extern template class Spectrogram<double>;

#endif // INCLUDE_AUDIOFILE_HPP_
