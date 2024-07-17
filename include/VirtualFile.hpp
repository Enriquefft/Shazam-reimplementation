#ifndef INCLUDE_VIRTUALFILE_HPP_
#define INCLUDE_VIRTUALFILE_HPP_
#include <sndfile.h>
#include <sstream>
#include <string>

namespace VirtualFile {

using stream = std::basic_istringstream<char>;

// Callback function to get the file length
inline auto get_filelen(void *user_data) -> sf_count_t {
  stream vf = stream(std::string(static_cast<char *>(user_data)));

  auto current_pos = vf.tellg();
  vf.seekg(0, std::ios::end);
  sf_count_t length = vf.tellg();
  vf.seekg(current_pos, std::ios::beg);
  return length;
}

// Callback function to seek to a specific position
inline auto seek(sf_count_t offset, int whence, void *user_data) -> sf_count_t {
  stream vf = stream(std::string(static_cast<char *>(user_data)));

  std::ios_base::seekdir dir;

  switch (whence) {
  case SEEK_SET:
    dir = std::ios::beg;
    break;
  case SEEK_CUR:
    dir = std::ios::cur;
    break;
  case SEEK_END:
    dir = std::ios::end;
    break;
  default:
    return -1;
  }

  vf.seekg(offset, dir);
  return vf.tellg();
}

// Callback function to read from the virtual file
inline auto read(void *ptr, sf_count_t count, void *user_data) -> sf_count_t {
  stream vf = stream(std::string(static_cast<char *>(user_data)));

  vf.read(static_cast<char *>(ptr), count);
  return vf.gcount();
}

// Callback function to write to the virtual file (not needed for read-only)
inline auto write(const void * /*ptr*/, sf_count_t /*count*/,
                  void * /*user_data*/) -> sf_count_t {
  // Writing is not supported in this example
  return 0;
}

// Callback function to tell the current position
inline auto tell(void *user_data) -> sf_count_t {
  stream vf = stream(std::string(static_cast<char *>(user_data)));
  return vf.tellg();
}

} // namespace VirtualFile
#endif // INCLUDE_VIRTUALFILE_HPP_
