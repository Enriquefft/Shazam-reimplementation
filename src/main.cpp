#include "Spotify.hpp"
#include <Spectrogram.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

auto main() -> int {
  std::string s;
  // std::cin >> s;

  Spectrogram<float> sp("assets/noisy.csv");
  sp.get_local_maximums();
  // std::cout << "Local Maxima found (processed " << sp.getX() << ','<<
  // sp.getY() <<") pixels" << std::endl;
  return 0;
}
