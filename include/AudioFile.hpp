#ifndef INCLUDE_AUDIOFILE_HPP_
#define INCLUDE_AUDIOFILE_HPP_

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <sndfile.h>

template <std::floating_point T> class Audio {

  void
  populate_audio_data(SF_INFO &sound_file_info, SNDFILE *sound_file,
                      const std::optional<float> &sample_rate = std::nullopt);

public:
  std::vector<T> m_audiodata;
  int m_sample_rate{};

  /// @brief create an Audio class instance from a wav_file
  explicit Audio(const std::filesystem::path &path,
                 const std::optional<float> &sample_rate = std::nullopt);

  /// @brief create an Audio class instance from a string, used when audio data
  /// comes from a formData
  explicit Audio(std::string &audio_data,
                 const std::optional<float> &sample_rate = std::nullopt);
};

extern template class Audio<float>;
extern template class Audio<double>;
extern template class Audio<long double>;

#endif // INCLUDE_AUDIOFILE_HPP_
