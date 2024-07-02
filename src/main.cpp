#include "Spotify.hpp"
#include <Spectrogram.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <AudioFile.hpp>

auto main() -> int {
  std::string s;
  Audio<float> a("assets/the_bidding.wav");
  Spectrogram<float> sp(a);
  sp.get_local_maximums();
  return 0;
}
