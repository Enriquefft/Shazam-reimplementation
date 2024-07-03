#include "AudioFile.hpp"
#include <filesystem>
#include <sndfile.h>
#include <stdexcept>

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

  sf_count_t nsamples = sound_file_info.frames; // on single channel its same
  m_sample_rate = sound_file_info.samplerate;

  m_audiodata.resize(static_cast<ulong>(nsamples));

  sf_count_t nread = 0;

  if constexpr (std::is_same_v<T, double>) {
    nread = sf_read_double(sound_file, m_audiodata.data(), nsamples);
  }
  if constexpr (std::is_same_v<T, float>) {
    nread = sf_read_float(sound_file, m_audiodata.data(), nsamples);
  }

  if (nread != nsamples) {
    sf_close(sound_file);
    throw std::runtime_error("Read " + std::to_string(nread) +
                             " samples, expected " + std::to_string(nsamples));
  }

  sf_close(sound_file);
}

// Explicit instantiation

template class Audio<float>;
template class Audio<double>;
