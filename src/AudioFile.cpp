#include "AudioFile.hpp"
#include <filesystem>
#include <sndfile.h>
#include <stdexcept>
#include <vector>

using std::vector;

template <std::floating_point T>
Audio<T>::Audio(const std::filesystem::path &path,
                const std::optional<float> &sample_rate) {

  if (sample_rate != std::nullopt) {
    throw std::invalid_argument("only native sample_rate is supported");
  }

  SF_INFO sound_file_info;

  SNDFILE *sound_file = sf_open(path.c_str(), SFM_READ, &sound_file_info);
  if (sound_file == nullptr) {
    throw std::invalid_argument("Could not read file: " + path.string());
  }

  size_t frame_count = sound_file_info.frames;
  size_t original_frame_count = frame_count * sound_file_info.channels;
  m_sample_rate = sound_file_info.samplerate;
  size_t channel_count = sound_file_info.channels;

  m_audiodata.resize(frame_count);

  sf_count_t nread = 0;

  vector<T> original_audio_frames(original_frame_count);

  if constexpr (std::is_same_v<T, double>) {
    nread = sf_read_double(sound_file, original_audio_frames.data(),
                           original_frame_count);
  }
  if constexpr (std::is_same_v<T, float>) {
    nread = sf_read_float(sound_file, original_audio_frames.data(),
                          original_frame_count);
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
