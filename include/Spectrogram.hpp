#ifndef INCLUDE_SPECTROGRAM_HPP_
#define INCLUDE_SPECTROGRAM_HPP_

#include "utils.hpp"
#include <complex>
#include <cstddef>  // for size_t
#include <cstdint>  // for int64_t
#include <optional> // for optional, nullopt, nullopt_t
#include <string>
#include <utility>
#include <vector>

template <std::floating_point T> class Audio;

// Currently window type and padding mode are limited
enum class WINDOW_FUNCT { HANN };
enum class PADDING_MODE { CONSTANT };
enum class PEAK_FUNCTION { MINLIST, GTN, MINLISTGTN};

struct Config {
    // to have the default value for stft and allow stft(audio) to work.
    // so that 'userspace' aint broken
    static constexpr auto FFT_WINDOW_DEFAULT = 4096;
    
    size_t FFT_WINDOW = FFT_WINDOW_DEFAULT;
    int MINLIST_SIZEX = 60;
    int MINLIST_SIZEY = 150;
    int GTN_SIZE = 5;
    double GTN_THRESHOLD = 1.2;
    PEAK_FUNCTION PEAK_ALGORITHM = PEAK_FUNCTION::MINLISTGTN;
    int HASH_BOXX = 150;
    int HASH_BOXY = 200;
    int HASH_BOX_DISPLACEMENT = 5;
};


/// @brief Class to generate and manipulate spectrograms from audio data.
template <std::floating_point T> class Spectrogram {
  using intensity_t = T;
  using hertz_t = uint32_t;
  using time_t = size_t;
private:
  /// @brief Structure to hold spectrogram data points.
  struct DataPoint {
    hertz_t hertz;         ///< Frequency bin.
    time_t time;           ///< Time bin.
    intensity_t intensity; ///< Intensity of the frequency at the given time.
  };

  std::vector<std::vector<T>> m_spectrogram; ///< 2D matrix for spectrogram.
  std::vector<DataPoint> m_features; ///< Extracted features from spectrogram.
  Config configuration;
  // hyperparameters config


  static auto
  block_wise_stft(std::vector<std::vector<std::complex<T>>> &stft_matrix,
                  const std::vector<std::vector<T>> &audiodata_frames,
                  const std::vector<std::vector<T>> &fft_window,
                  const auto &n_columns, const size_t &off_start)
      -> std::pair<std::vector<std::vector<std::complex<T>>>, size_t>;
  static auto
  padding_stft(std::vector<std::vector<std::complex<T>>> &stft_matrix,
               const std::vector<std::vector<T>> &audiodata_frames_pre,
               const std::vector<std::vector<T>> &audiodata_frames_post,
               const std::vector<std::vector<T>> &fft_window)
      -> std::pair<std::vector<std::vector<std::complex<T>>>, size_t>;

  /// @brief Call the window function.
  /// @param window Type of window function.
  /// @param n_points Number of points in the window.
  static auto get_window(const WINDOW_FUNCT &window, const size_t &n_points)
      -> std::vector<T>;

  /// @brief Generate a Hann window.
  /// @param n_points Number of points in the window.
  /// @return Hann window.
  static auto hann(size_t n_points) -> std::vector<T>;

  /// @brief Center and pad a given data.
  /// @param data The data to pad.
  /// @param target_size The target size after padding.
  /// @param padding_mode Padding strategy to use.
  /// @return Padded data.
  static auto
  pad_center(const std::vector<T> &data, const size_t &target_size,
             const PADDING_MODE &padding_mode = PADDING_MODE::CONSTANT)
      -> std::vector<T>;

  /// @brief Pad the data to a specified width.
  /// @param data The data to pad.
  /// @param pad_width Pair of padding sizes for each end.
  /// @param padding_mode Padding strategy to use.
  /// @param constant_value Value to use for constant padding.
  /// @return Padded data.
  static auto pad(const std::vector<T> &data,
                  const std::pair<int64_t, int64_t> &pad_width,
                  const PADDING_MODE &padding_mode = PADDING_MODE::CONSTANT,
                  const T &constant_value = T{}) -> std::vector<T>;

  /// @brief Expand the data to a specified dimension.
  /// @param data The data to expand.
  /// @param target_dim The target dimension.
  static auto expand_to(const std::vector<T> &data, const size_t &target_dim);

  using spdata_t = std::vector<std::vector<intensity_t>>;
  using spcol_t = std::vector<intensity_t>;
  using CritSet_t = std::vector<DataPoint>;
  //

  //================================= MAXIMA finding

  /// @brief Get local maxima by maxfiltering subtracting original and finding
  /// zeroes
  /// @param neigh size of sliding window
  /// @return local maxima points
  auto maxima_minlist_algorithm(int neighx,int neighy) -> std::vector<DataPoint>;
  /// @brief O(n) implementation of maxima_MINLIST_algorithm. May have slightly
  /// different results
  /// @param neigh size of sliding windowlocal maxima points
  /// @return local maxima points
  auto maxima_minlist_algorithm_optimized(int neigh, int neighy) -> std::vector<DataPoint>;
  /// @brief Find maximum points by first pulling candidates using optimized
  /// MINLIST and culling them with GTN
  /// @param maxfilter_s size of maxfilter window
  /// @param gtn_s size of GTN window
  /// @param thresh How much above average must a local maxima be.
  /// @return local maxima points
  auto maxima_minlistgcn_algorithm(int maxfilter_sx,int maxfilter_sy, int gtn_s,
                                   intensity_t thresh)
      -> std::vector<DataPoint>;
  /// @brief get local maxima defined as all points that are greater that those
  /// of their neighborhood
  auto maxima_gtn_algorithm(int neighbourhood, T thrsh)
      -> std::vector<DataPoint>;

  inline auto is_max_in_neighborhood(size_t max_x, size_t max_y,
                                     size_t current_x, size_t current_y,
                                     int neighborhood_size,
                                     intensity_t threshold,
                                     const spdata_t &sp_data) -> bool;
  inline auto peak_filter_minlist(const intensity_t &maxd,
                                  const intensity_t &spd) -> bool;
  inline auto peak_filter_minlist_gtn(const size_t &current_x,
                                      const size_t &current_y,
                                      const spdata_t &main_filter,
                                      const spdata_t &sp_data,
                                      const size_t &max_x, const size_t &max_y,
                                      const int &neighborhood_size,
                                      const intensity_t &threshold) -> bool;
  auto max_in_neighborhood(size_t max_x, size_t max_y, unsigned current_x,
                           unsigned current_y, int neigh_size_x, int neigh_size_y,
                           const spdata_t &sp_data) -> intensity_t;

  void maxfilter_x(spdata_t &maxfiltered_spectrogram,
                   const spdata_t &spectrogram, size_t sp_x, size_t sp_y,
                   int neigh);
  void maxfilter_y(spdata_t &maxfiltered_spectrogram, size_t sp_x, size_t sp_y,
                   int neigh);
  /// ========================= HASHING =====================
  auto generate_hashes_naive(
      std::vector<typename Spectrogram<T>::DataPoint> &pivots,
      std::vector<typename Spectrogram<T>::DataPoint> &localmaxima,
      size_t box_height, size_t box_width, size_t box_displacement)
      -> std::vector<std::pair<uint32_t, size_t>>;

  auto select_pivots_naive(
      const std::vector<typename Spectrogram<T>::DataPoint> &pts)
      -> std::vector<typename Spectrogram<T>::DataPoint>;

public:
  /// @brief Constructor to generate the Spectrogram from audio data.
  /// @param audio The audio data to use.
  explicit Spectrogram(const Audio<T> &audio);

  explicit Spectrogram(const Audio<T> &audio, Config& cfg);

  /// @brief Read a spectrogram from a CSV like a monochrome image. Delete this
  /// on final integration
  /// @param csvname filename of the csv to read!
  explicit Spectrogram(const std::string &csvname);

  auto get_x() -> size_t;
  auto get_y() -> size_t;
  auto get_feature_count() -> size_t;

  auto get_hashes() -> std::vector<std::pair<uint32_t, size_t>>;

  /// @brief Get the generated spectrogram.
  /// @return 2D matrix of the spectrogram.
  [[nodiscard]] auto get_spectrogram() const
      -> const std::vector<std::vector<T>> &;

  /// @brief Extract local maximum features from the spectrogram.
  /// @return Vector of data points representing local maximums.
  auto get_local_maximums() -> std::vector<DataPoint>;

  ///@brief Get the db_normalized spectrogram
  static auto normalize(const matrix_t<T> &spectrogram)
      -> std::vector<std::vector<T>>;

  /// @brief Short-time Fourier transform (STFT) calculation.
  /// @param audio The audio data to transform.
  /// @param n_fft Number of FFT components.
  /// @param hop_length Number of audio samples between adjacent STFT columns.
  /// @param window_length Each frame of audio is windowed by window_length.
  /// @param window Type of window function to use.
  /// @param center If true, pads the signal to center the frame.
  /// @param padding_mode Padding strategy to use.
  static auto stft(const Audio<T> &audio, const size_t &n_fft = Config::FFT_WINDOW_DEFAULT,
                   const std::optional<size_t> &hop_length = std::nullopt,
                   const std::optional<size_t> &window_length = std::nullopt,
                   const WINDOW_FUNCT &window = WINDOW_FUNCT::HANN,
                   bool center = true,
                   const PADDING_MODE &padding_mode = PADDING_MODE::CONSTANT)
      -> std::vector<std::vector<std::complex<T>>>;

  /// @brief Frame a given audio data.
  /// @param audiodata The audio data to frame.
  /// @param frame_length Length of each frame.
  /// @param hop_length Number of samples between frames.
  /// @return 2D matrix with framed audio data.
  static auto frame(const std::vector<T> &audiodata, size_t frame_length,
                    size_t hop_length) -> std::vector<std::vector<T>>;
};

template <std::floating_point T>
explicit Spectrogram(const Audio<T> &audio) -> Spectrogram<T>;

extern template class Spectrogram<float>;
extern template class Spectrogram<double>;
extern template class Spectrogram<long double>;

#endif // INCLUDE_SPECTROGRAM_HPP_
