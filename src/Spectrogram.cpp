#include "Spectrogram.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <iostream>
#include <sndfile.h>

using std::floating_point;
using std::pair;
using std::vector;

template <typename... Args> void print(std::string_view fmt, Args &&...args) {
  std::cout << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

template <floating_point T>
Spectrogram<T>::Spectrogram(const Audio<T> &audio)
    : m_spectrogram({}), m_features({}) {

  stft(audio);
}

template <floating_point T>
auto Spectrogram<T>::get_spectrogram() -> vector<vector<T>> {
  return m_spectrogram;
}

template <floating_point T>
auto Spectrogram<T>::get_local_maximums() -> vector<DataPoint> {
  return {};
}

template <floating_point T>
auto Spectrogram<T>::get_window(const WINDOW_FUNCT &window,
                                const size_t &n_points) {
  switch (window) {
  case WINDOW_FUNCT::HANN:
    return hann(n_points);
  }
}

template <floating_point T>
auto Spectrogram<T>::empty_like(std::pair<size_t, size_t> dimensions)
    -> vector<vector<T>> {
  size_t rows = dimensions.first;
  size_t cols = dimensions.second;

  return vector<vector<T>>(rows, vector<T>(cols));
}

template <floating_point T>
auto Spectrogram<T>::frame(const std::vector<T> &audiodata, size_t frame_length,
                           size_t hop_length) -> std::vector<std::vector<T>> {
  if (audiodata.size() < static_cast<size_t>(frame_length)) {
    throw std::invalid_argument(
        "Input is too short for the given frame length.");
  }

  // Calculate the number of frames
  auto num_frames = 1 + (audiodata.size() - frame_length) / hop_length;
  // auto num_frames = static_cast<int>(x.size()) / hop_length;

  // Initialize the output vector
  std::vector<std::vector<T>> frames(num_frames, std::vector<T>(frame_length));

  // Populate the frames
  for (size_t i = 0; i < num_frames; ++i) {
    for (size_t j = 0; j < frame_length; ++j) {
      frames[i][j] = audiodata[i * hop_length + j];
    }
  }

  return frames;
}

template <floating_point T>
auto Spectrogram<T>::hann(size_t n_points) -> vector<T> {

  if (n_points == 0) {
    return {};
  }

  auto window_length = n_points + 1;

  vector<T> window;
  window.reserve(window_length);

  for (size_t win_elem = 0; win_elem < window_length; ++win_elem) {

    window.push_back(
        (1 / 2) *
        (1 - std::cos(2 * static_cast<T>(std::numbers::pi) *
                      static_cast<T>(win_elem) / static_cast<T>(n_points))));
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
  //   std::reverse_copy(src.begin(), src.begin() + left_pad, dest.begin());
  //   std::copy(src.begin(), src.end(), dest.begin() + left_pad);
  //   std::reverse_copy(src.end() - right_pad, src.end(), dest.end() -
  //   right_pad);
  // };
  //
  // auto pad_symmetric = [&](const vector<T> &src, vector<T> &dest,
  //                          int64_t left_pad, int64_t right_pad) {
  //   std::copy(src.begin() + 1, src.begin() + 1 + left_pad, dest.begin());
  //   std::copy(src.begin(), src.end(), dest.begin() + left_pad);
  //   std::copy(src.end() - right_pad - 1, src.end() - 1, dest.end() -
  //   right_pad);
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

  vector<vector<T>> expanded(target_dim - 1, vector<T>(data.size(), T()));

  // Copy the original data to the expanded vector
  for (size_t i = 0; i < data.size(); ++i) {
    expanded[0][i] = data[i];
  }

  return expanded;
}

template <floating_point T>
void Spectrogram<T>::stft(const Audio<T> &audio, const size_t &n_fft,
                          const std::optional<size_t> &hop_length,
                          const std::optional<size_t> &window_length,
                          const WINDOW_FUNCT &window, bool center,
                          const PADDING_MODE &padding_mode) {
  print("calling stft with: {}", center);

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

  // Set up the padding array to be empty, and we'll fix the target dimension

  pair<size_t, size_t> padding;

  //
  // How many frames depend on left padding ?
  auto start_k =
      static_cast<size_t>(std::ceil(n_fft / 2 / effective_hop_length));

  // What's the first frame that depends on extra right-padding?

  auto tail_k =
      (audiodata.size() + n_fft / 2 - n_fft) / (effective_hop_length + 1);

  size_t start = 0;
  size_t extra = 0;

  vector<vector<T>> audiodata_frames_post;
  vector<vector<T>> audiodata_frames_pre;

  if (tail_k <= start_k) {
    start = 0;
    extra = 0;
    padding = {n_fft / 2, n_fft / 2};
    audiodata = pad(audiodata, padding, padding_mode);
  } else {
    // If tail and head do not overlap, then we can implement padding on each
    // part separately # and avoid a full copy-pad

    // "Middle" of the signal starts here, and does not depend on head
    start = start_k * effective_hop_length - n_fft / 2;
    padding = {n_fft / 2, 0};

    auto audiodata_pre_idx =
        (start_k - 1) * effective_hop_length - n_fft / 2 + n_fft + 1;

    if (audiodata_pre_idx > audiodata.size()) {
      throw std::runtime_error("y_pre end idx has wrong size");
    }
    std::vector<T> sliced_audiodata(
        audiodata.begin(),
        audiodata.begin() + static_cast<int64_t>(audiodata_pre_idx));
    auto audiodata_pre = pad(sliced_audiodata, padding, padding_mode);

    audiodata_frames_pre = frame(audiodata_pre, n_fft, effective_hop_length);

    // # Trim this down to the exact number of frames we should have
    audiodata_frames_pre = std::vector(audiodata_frames_pre.begin(),
                                       audiodata_frames_pre.begin() +
                                           static_cast<int64_t>(start_k));

    extra = audiodata_frames_pre.size();

    // Determine if we have any frames that will fit inside the tail pad
    if (tail_k * effective_hop_length - n_fft / 2 + n_fft <=
        audiodata.size() + n_fft / 2) {

      auto audiodata_post_idx = tail_k * effective_hop_length - n_fft / 2;

      auto audiodata_post =
          pad(std::vector(audiodata.begin(),
                          audiodata.begin() +
                              static_cast<int64_t>(audiodata_post_idx)),
              padding, padding_mode);

      audiodata_frames_post =
          frame(audiodata_post, n_fft, effective_hop_length);
      //     How many extra frames do we have from the tail?
      extra += audiodata_frames_post.size();
    } else {
      // In this event, the first frame that touches tail padding would run off
      // the end of the padded array We'll circumvent this by allocating an
      // empty frame buffer for the tail
      //  this keeps the subsequent logic simple

      pair<size_t, size_t> post_shape = {audiodata_frames_pre.size(), 0};

      audiodata_frames_post = empty_like(post_shape);
    }
  }
  // audiodata_frames = frame(y[..., start:], frame_length=n_fft,
  // hop_length=hop_length)
  auto audiodata_frames =
      frame(vector(audiodata.begin(),
                   audiodata.begin() + static_cast<int64_t>(start)),
            n_fft, effective_hop_length

      );
  pair<size_t, size_t> shape = {1 + n_fft / 2, extra};
  auto stft_matrix = empty_like(shape);

  // Fill in the warm-up
  // size_t off_start = 0;
  if (center && extra > 0) {
    // off_start = audiodata_frames_pre.size();
  }

  m_spectrogram = stft_matrix;
}

// Explicit instantiation

template class Spectrogram<float>;
template class Spectrogram<double>;
