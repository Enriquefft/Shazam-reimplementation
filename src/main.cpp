#include "Spotify.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

auto main() -> int {

  spdlog::info("Hello world");
  Spotify spot;
  auto tracks = spot.get_track_names("3p6W0GFxrobpbCet8uQTYk");
  for (const auto &track : tracks) {
    std::cout << track << '\n';
  }
  return 0;
}
