#include "Spotify.hpp"
#include <iostream>
#include <AudioFile.hpp>
#include <spdlog/spdlog.h>

auto main() -> int {
  std::string s;
  std::cin >> s;

  Spectrogram sp(s);
  for (auto& i:sp.get_local_maximums())
  {
    std::cout << i.time << ", " << i.hertz << std::endl;
  }
  return 0;
}
