#include "AudioFile.hpp"
#include "utils.hpp"
#include <filesystem>
#include <sndfile.h>
#include <stdexcept>
#include <vector>

using std::vector;

template <std::floating_point T>
Audio<T>::Audio(const std::filesystem::path &path,
                const std::optional<float> &sample_rate) {

  info = true;

  if (sample_rate != std::nullopt) {
    throw std::invalid_argument("only native sample_rate is supported");
  }

  SF_INFO sound_file_info;

  SNDFILE *sound_file = sf_open(path.c_str(), SFM_READ, &sound_file_info);
  if (sound_file == nullptr) {
    throw std::invalid_argument("Could not read file: " + path.string());
  }

  auto channel_count = static_cast<size_t>(sound_file_info.channels);
  auto frame_count = static_cast<size_t>(sound_file_info.frames);
  size_t original_frame_count = frame_count * channel_count;
  m_sample_rate = sound_file_info.samplerate;

  m_audiodata.resize(frame_count);

  vector<T> original_audio_frames(original_frame_count);

  if constexpr (std::is_same_v<T, double>) {
    sf_read_double(sound_file, original_audio_frames.data(),
                   static_cast<int64_t>(original_frame_count));
  }
  if constexpr (std::is_same_v<T, float>) {
    sf_read_float(sound_file, original_audio_frames.data(),
                  static_cast<int64_t>(original_frame_count));
  }
  if constexpr (std::is_same_v<T, long double>) {
    vector<double> double_audio_frames(original_frame_count);
    sf_read_double(sound_file, double_audio_frames.data(),
                   static_cast<int64_t>(original_frame_count));

    for (size_t i = 0; i < original_frame_count; i++) {
      original_audio_frames.at(i) =
          static_cast<long double>(double_audio_frames.at(i));
    }
  }

  for (size_t i = 0; i < frame_count; i++) {
    m_audiodata.at(i) = 0;
    for (size_t j = 0; j < channel_count; j++) {
      m_audiodata.at(i) += original_audio_frames.at(i * channel_count + j);
    }

    m_audiodata.at(i) /= static_cast<T>(channel_count);
  }

  sf_close(sound_file);
}

// Explicit instantiation
template class Audio<float>;
template class Audio<double>;
template class Audio<long double>;
