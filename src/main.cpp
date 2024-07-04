#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

auto main() -> int {

  auto songs = {"a", "b"};

  for (auto s : songs) {

    Audio<float> a("assets/the_bidding.wav");

    Spectrogram<float> sp(a);

    sp.get_local_maximums();
    std::cout << "Local Maxima found (processed " << sp.get_x() << ','
              << sp.get_y() << ") pixels" << '\n';

    // db.insert(sp.get_features)
  }

  // db.search(ne_song)

  return 0;
}
