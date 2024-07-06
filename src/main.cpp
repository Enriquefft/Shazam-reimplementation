#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

auto main() -> int {

  std::vector<std::string> songs = {"assets/the_bidding.wav"};

  for (const auto &song : songs) {

    Audio<float> audio(song);

    Spectrogram<float> spectrogram(audio);

    spectrogram.get_local_maximums();
    auto local_max = spectrogram.get_local_maximums();
    auto spec = spectrogram.get_spectrogram();
    // std::cout << spec.size() << ',' << spec[0].size() << '\n';
    for (auto max : local_max) {
      std::cout << max.time << ',' << max.hertz << '\n';
    }

    // db.insert(sp.get_features)
  }

  // db.search(ne_song)

  return 0;
}
