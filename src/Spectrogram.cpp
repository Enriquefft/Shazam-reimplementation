#include "Spectrogram.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <numbers>
#include <stdexcept>

#include "AudioFile.hpp"
#include "fft.hpp"
#include "utils.hpp"

using std::floating_point;
using std::pair;
using std::vector;

constexpr auto MAX_MEM_BLOCK = binpow(2, 8) * binpow(2, 10);

template <floating_point T>
auto Spectrogram<T>::block_wise_stft(matrix_t<std::complex<T>> &stft_matrix,
                                     const matrix_t<T> &audiodata_frames,
                                     const matrix_t<T> &fft_window,
                                     const auto &n_columns,
                                     const size_t &off_start)
    -> pair<matrix_t<std::complex<T>>, size_t> {

  size_t y_frames_cols = audiodata_frames.at(0).size();

  for (size_t bl_s = 0; bl_s < y_frames_cols; bl_s += n_columns) {

    size_t bl_t = std::min(bl_s + n_columns, y_frames_cols);

    auto block_frames = slice_cols(audiodata_frames, bl_s, bl_t);

    auto fft_result = row_dft(multiply(fft_window, block_frames));

    for (size_t i = 0; i < stft_matrix.size(); ++i) {
      for (size_t j = bl_s + off_start; j < bl_t + off_start; ++j) {
        stft_matrix[i][j] = fft_result[i][j - bl_s - off_start];
      }
    }
  }

  return {stft_matrix, off_start};
}

template <floating_point T>
auto Spectrogram<T>::padding_stft(matrix_t<std::complex<T>> &stft_matrix,
                                  const matrix_t<T> &audiodata_frames_pre,
                                  const matrix_t<T> &audiodata_frames_post,
                                  const matrix_t<T> &fft_window)
    -> pair<matrix_t<std::complex<T>>, size_t> {

  matrix_t<std::complex<T>> fft_pre =
      row_dft(multiply(fft_window, audiodata_frames_pre));

  vector<vector<std::complex<T>>> fft_post =
      row_dft(multiply(fft_window, audiodata_frames_post));

  size_t off_start = audiodata_frames_pre.at(0).size();
  size_t off_end = audiodata_frames_post.at(0).size();

  // Copy fft_pre into stft_matrix at the beginning
  for (size_t i = 0; i < stft_matrix.size(); ++i) {
    for (size_t j = 0; j < off_start; ++j) {
      stft_matrix.at(i).at(j) = fft_pre.at(i).at(j);
    }
  }

  // Copy fft_post into stft_matrix at the end
  if (off_end > 0) {
    size_t stft_cols = stft_matrix.at(0).size();
    for (size_t i = 0; i < stft_matrix.size(); ++i) {
      for (size_t j = 0; j < off_end; ++j) {
        stft_matrix.at(i).at(stft_cols - off_end + j) = fft_post.at(i).at(j);
      }
    }
  }

  return {stft_matrix, off_start};
}

template <floating_point T>
Spectrogram<T>::Spectrogram(const Audio<T> &audio)
    : m_spectrogram({}), m_features({}) {

  auto stft_matrix = stft(audio);
  m_spectrogram = abs(stft_matrix);

  for (size_t i = 0; i < m_spectrogram.size(); i++) {
    for (size_t j = 0; j < m_spectrogram[0].size(); j++) {
      m_spectrogram.at(i).at(j) = std::log(m_spectrogram[i][j] + 1);
    }
  }
}

template <floating_point T>
Spectrogram<T>::Spectrogram(const std::string &csvname) {
  size_t width = 0;
  size_t height = 0;

  std::string s;
  std::ifstream csv(csvname);
  if (!csv.is_open()) {
    std::cerr << "Error opening spectrogram.csv" << '\n';
    return;
  }

  // Read dimensions from the first line
  getline(csv, s);
  std::stringstream dim_ss(s);
  char delimiter = 0;
  dim_ss >> width >> delimiter >> height;

  // Resize the spectrogram matrix
  m_spectrogram.resize(height, std::vector<intensity_t>(width, 0));

  // Read spectrogram data from the rest of the file
  for (size_t i = 0; i < height; ++i) {
    if (!getline(csv, s)) {
      std::cerr << "Error: Insufficient data in spectrogram.csv" << '\n';
      return;
    }
    std::stringstream line_ss(s);
    for (size_t j = 0; j < width; ++j) {
      std::string intensity_str;
      if (!getline(line_ss, intensity_str, ',')) {
        std::cerr << "Error: Insufficient data in row " << i + 1 << '\n';
        return;
      }
      try {
        m_spectrogram[i][j] =
            static_cast<intensity_t>(std::stoi(intensity_str));
      } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid argument: " << e.what() << '\n';
        return;
      } catch (const std::out_of_range &e) {
        std::cerr << "Out of range error: " << e.what() << '\n';
        return;
      }
    }
  }

  csv.close();
}

template <floating_point T>
auto Spectrogram<T>::get_spectrogram() -> matrix_t<T> {
  return m_spectrogram;
}

constexpr auto MIN_THRESHOLD = 1e-10;
template <floating_point T>
auto Spectrogram<T>::normalize(const matrix_t<T> &spectrogram)
    -> std::vector<std::vector<T>> {

  matrix_t<T> normalized_spectrogram;
  normalized_spectrogram.reserve(spectrogram.size());

  T max_normalized_elem;
  std::ranges::transform(
      spectrogram, std::back_inserter(normalized_spectrogram),
      [&max_normalized_elem](const auto &row) {
        std::vector<T> log_squared_row;
        log_squared_row.reserve(row.size());

        std::ranges::transform(
            row, std::back_inserter(log_squared_row),
            [&max_normalized_elem](const auto &elem) {
              T new_elem =
                  10 * std::log10(std::max(T(MIN_THRESHOLD), elem * elem));
              if (new_elem > max_normalized_elem) {
                max_normalized_elem = new_elem;
              }
              return new_elem;
            });

        return log_squared_row;
      });

  max_normalized_elem -= 80;
  // Apply upper bound
  std::ranges::transform(
      normalized_spectrogram.begin(), normalized_spectrogram.end(),
      normalized_spectrogram.begin(), [&max_normalized_elem](auto &row) {
        std::ranges::transform(row.begin(), row.end(), row.begin(),
                               [&max_normalized_elem](const auto &elem) {
                                 return std::max(elem, max_normalized_elem);
                               });

        return row;
      });

  return normalized_spectrogram;
}

template <typename T>
inline auto slice_matrix(const matrix_t<T> &matrix, size_t off_start)
    -> matrix_t<T> {
  matrix_t<T> sliced_matrix;

  for (const auto &row : matrix) {
    if (off_start <= row.size()) {
      sliced_matrix.push_back(vector<T>(row.begin(), row.begin() + off_start));
    } else {
      // Handle case where off_start is greater than the row size
      sliced_matrix.push_back(row); // Or handle this case as needed
    }
  }

  return sliced_matrix;
}

template <floating_point T>
auto Spectrogram<T>::get_window(const WINDOW_FUNCT &window,
                                const size_t &n_points) -> vector<T> {
  switch (window) {
  case WINDOW_FUNCT::HANN: {
    return hann(n_points);
  }
  }
  throw std::invalid_argument("invalid WINDOW_FUNCT");
}

template <floating_point T>
auto Spectrogram<T>::frame(const vector<T> &audiodata, size_t frame_length,
                           size_t hop_length) -> matrix_t<T> {

  if (audiodata.size() < frame_length) {
    throw std::invalid_argument(
        "Input is too short for the given frame length.");
  }

  // Calculate the number of frames
  size_t num_frames = 1 + (audiodata.size() - frame_length) / hop_length;

  // Initialize the output vector
  auto frames = generate_matrix<T>({frame_length, num_frames});

  // Populate the frames
  for (size_t i = 0; i < num_frames; ++i) {
    auto start = i * hop_length;

    for (size_t j = 0; j < frame_length; j++) {
      frames.at(j).at(i) = audiodata.at(start + j);
    }
  }

  return frames;
}

template <floating_point T>
auto Spectrogram<T>::hann(size_t n_points) -> vector<T> {
  if (n_points == 0) {
    return {};
  }

  auto window_length = n_points;

  vector<T> window;
  window.reserve(window_length);

  for (size_t win_elem = 0; win_elem < window_length; win_elem++) {

    auto win_value =
        T(.5) *
        (1 - std::cos(2 * static_cast<T>(std::numbers::pi) *
                      static_cast<T>(win_elem) / static_cast<T>(n_points - 1)));
    window.push_back(win_value);
  }

  return window;
}

template <floating_point T>
auto Spectrogram<T>::pad(const vector<T> &data,
                         const pair<int64_t, int64_t> &pad_width,
                         const PADDING_MODE &padding_mode,
                         const T &constant_value) -> vector<T> {
  size_t target_size = data.size() + static_cast<size_t>(pad_width.first) +
                       static_cast<size_t>(pad_width.second);

  vector<T> result(target_size);

  auto pad_constant = [&](const vector<T> &src, vector<T> &dest,
                          pair<int64_t, int64_t> padding, T value) {
    std::fill(dest.begin(), dest.begin() + padding.first, value);
    std::copy(src.begin(), src.end(), dest.begin() + padding.first);
    std::fill(dest.end() - padding.second, dest.end(), value);
  };

  // auto pad_reflect = [&](const vector<T> &src, vector<T> &dest,
  //                        int64_t left_pad, int64_t right_pad) {
  //   std::reverse_copy(src.begin(), src.begin() + left_pad,
  //   dest.begin()); std::copy(src.begin(), src.end(), dest.begin() +
  //   left_pad); std::reverse_copy(src.end() - right_pad, src.end(),
  //   dest.end() - right_pad);
  // };
  //
  // auto pad_symmetric = [&](const vector<T> &src, vector<T> &dest,
  //                          int64_t left_pad, int64_t right_pad) {
  //   std::copy(src.begin() + 1, src.begin() + 1 + left_pad,
  //   dest.begin()); std::copy(src.begin(), src.end(), dest.begin() +
  //   left_pad); std::copy(src.end() - right_pad - 1, src.end() - 1,
  //   dest.end() - right_pad);
  // };

  switch (padding_mode) {
  case PADDING_MODE::CONSTANT:
    pad_constant(data, result, pad_width, constant_value);
    break;
  }

  return result;
}
template <floating_point T>
auto Spectrogram<T>::pad_center(const vector<T> &data,
                                const size_t &target_size,
                                const PADDING_MODE &padding_mode) -> vector<T> {
  if (target_size <= data.size()) {
    return data;
  }

  // Calculate the amount of padding on the left and right
  size_t total_padding = target_size - data.size();
  size_t left_padding = total_padding / 2;
  size_t right_padding = total_padding - left_padding;

  return pad(data, {left_padding, right_padding}, padding_mode);
}

template <floating_point T>
auto Spectrogram<T>::expand_to(const vector<T> &data,
                               const size_t &target_dim) {
  if (target_dim < 2) {
    throw std::invalid_argument("ndim must be at least 2 for this example.");
  }

  matrix_t<T> expanded(data.size(), vector<T>(target_dim - 1, T()));

  // Copy the original data to the expanded vector
  for (size_t i = 0; i < data.size(); ++i) {
    expanded.at(i).at(target_dim - 2) = data.at(i);
  }

  return expanded;
}

template <floating_point T>
auto Spectrogram<T>::stft(const Audio<T> &audio, const size_t &n_fft,
                          const std::optional<size_t> &hop_length,
                          const std::optional<size_t> &window_length,
                          const WINDOW_FUNCT &window, bool center,
                          const PADDING_MODE &padding_mode)
    -> matrix_t<std::complex<T>> {

  auto effectve_window_length = window_length.value_or(n_fft);
  auto effective_hop_length = hop_length.value_or(effectve_window_length / 4);

  auto fft_window = get_window(window, n_fft);

  auto audiodata = audio.m_audiodata;

  // Pad the window out to n_fft size
  fft_window = pad_center(fft_window, n_fft);

  // Reshape so that the window can be broadcast
  auto expanded_fft_window = expand_to(fft_window, 2);

  if (n_fft > audiodata.size()) {
    throw std::runtime_error("n_fft is to large for audio input");
  }

  // Set up the padding array to be empty, and we'll fix the target
  // dimension

  pair<size_t, size_t> padding;

  //
  // How many frames depend on left padding ?
  auto start_k =
      static_cast<size_t>(std::ceil(n_fft / 2 / effective_hop_length));

  // What's the first frame that depends on extra right-padding?

  auto tail_k =
      (audiodata.size() + n_fft / 2 - n_fft) / effective_hop_length + 1;

  size_t start = 0;
  size_t extra = 0;

  matrix_t<T> audiodata_frames_post;
  matrix_t<T> audiodata_frames_pre;

  if (tail_k <= start_k) {
    start = 0;
    extra = 0;
    padding = {n_fft / 2, n_fft / 2};
    audiodata = pad(audiodata, padding, padding_mode);
  } else {
    // If tail and head do not overlap, then we can implement padding on
    // each part separately and avoid a full copy-pad

    // "Middle" of the signal starts here, and does not depend on head
    start = start_k * effective_hop_length - n_fft / 2;
    padding = {n_fft / 2, 0};

    auto audiodata_pre_idx =
        (start_k - 1) * effective_hop_length - n_fft / 2 + n_fft + 1;

    if (audiodata_pre_idx > audiodata.size()) {
      throw std::runtime_error("y_pre end idx has wrong size");
    }
    vector<T> sliced_audiodata(audiodata.begin(),
                               audiodata.begin() +
                                   static_cast<int64_t>(audiodata_pre_idx));

    auto audiodata_pre = pad(sliced_audiodata, padding, padding_mode);

    audiodata_frames_pre = frame(audiodata_pre, n_fft, effective_hop_length);

    // Trim this down to the exact number of frames we should have
    slice_cols(audiodata_frames_pre, 0, start_k);

    extra = audiodata_frames_pre.at(0).size();

    // Determine if we have any frames that will fit inside the tail pad
    if (tail_k * effective_hop_length - n_fft / 2 + n_fft <=
        audiodata.size() + n_fft / 2) {

      auto audiodata_post_idx =
          static_cast<int64_t>(tail_k * effective_hop_length - n_fft / 2);

      auto audiodata_trimmed_post = audiodata;
      audiodata_trimmed_post.erase(audiodata_trimmed_post.begin(),
                                   audiodata_trimmed_post.begin() +
                                       audiodata_post_idx);

      auto audiodata_post = pad(audiodata_trimmed_post, padding, padding_mode);

      info = true;

      audiodata_frames_post =
          frame(audiodata_post, n_fft, effective_hop_length);

      info = false;

      // How many extra frames do we have from the tail?
      extra += audiodata_frames_post.at(0).size();
    } else {

      // In this event, the first frame that touches tail padding would
      // run off the end of the padded array We'll circumvent this by
      // allocating an empty frame buffer for the tail
      //  this keeps the subsequent logic simple

      pair<size_t, size_t> post_shape = {audiodata_frames_pre.size(), 0};

      audiodata_frames_post = generate_matrix<T>(post_shape);
    }
  }

  info = true;

  auto audiodata_frames = frame(
      vector(audiodata.begin() + static_cast<int64_t>(start), audiodata.end()),
      n_fft, effective_hop_length);

  pair<size_t, size_t> shape = {1 + n_fft / 2,
                                extra + audiodata_frames.at(0).size()};

  matrix_t<std::complex<T>> stft_matrix =
      generate_matrix<std::complex<T>>(shape);

  // Proccesss the extra padding frames with fft if needed
  size_t off_start = 0;
  if (center && extra > 0) {

    auto pad_stft = padding_stft(stft_matrix, audiodata_frames_pre,
                                 audiodata_frames_post, expanded_fft_window);
    stft_matrix = pad_stft.first;
    off_start = pad_stft.second;
  }

  // maximize the columns per block
  auto n_columns = MAX_MEM_BLOCK / (audiodata_frames.size() * sizeof(T));

  // Process the main audiodata with a block-based sliding window
  auto block_stft = block_wise_stft(stft_matrix, audiodata_frames,
                                    expanded_fft_window, n_columns, off_start);
  stft_matrix = block_stft.first;
  off_start = block_stft.second;

  return stft_matrix;
}

// Explicit instantiation

template class Spectrogram<float>;
template class Spectrogram<double>;
template class Spectrogram<long double>;
