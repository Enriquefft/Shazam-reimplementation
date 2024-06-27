#ifndef INCLUDE_AUDIOFILE_HPP_
#define INCLUDE_AUDIOFILE_HPP_

#include <sndfile.h>
#include <string>
#include <vector>

class Audio {
public:
  std::vector<int16_t> m_samples;
  int m_rate;

  // Read a single-track audio file
  explicit Audio(const std::string &path);
};

#endif // INCLUDE_AUDIOFILE_HPP_
