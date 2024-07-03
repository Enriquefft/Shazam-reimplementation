#ifndef INCLUDE_AUDIOFILE_HPP_
#define INCLUDE_AUDIOFILE_HPP_

#include <filesystem>
#include <optional>
#include <vector>

template <std::floating_point T> class Audio {
public:
  std::vector<T> m_audiodata;
  int m_sample_rate;

  /// @brief create an Audio class instance from a wav_file
  explicit Audio(const std::filesystem::path &path,
                 const std::optional<float> &sample_rate = std::nullopt);
};

extern template class Audio<float>;
extern template class Audio<double>;

#endif // INCLUDE_AUDIOFILE_HPP_
