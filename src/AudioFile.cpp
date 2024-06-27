#include "AudioFile.hpp"
#include <stdexcept>

// Construir audio desde un archivo usando libsndfile
Audio::Audio(const std::string &path) {
  SF_INFO sound_file_info;
  SNDFILE *sound_file = sf_open(path.c_str(), SFM_READ, &sound_file_info);
  if (sound_file == nullptr) {
    throw std::invalid_argument("Could not read file: " + path);
  }

  sf_count_t nsamples = sound_file_info.frames; // on single channel its same
  m_rate = sound_file_info.samplerate;

  m_samples.resize(static_cast<ulong>(nsamples));
  sf_count_t nread = sf_read_short(sound_file, m_samples.data(), nsamples);

  if (nread != nsamples) {
    sf_close(sound_file);
    throw std::runtime_error("Read " + std::to_string(nread) +
                             " samples, expected " + std::to_string(nsamples));
  }
}
