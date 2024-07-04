#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

auto main() -> int {
  Audio<float> a("assets/the_bidding.wav");
  Spectrogram<float> sp(a);
  auto lm = sp.get_local_maximums();
  auto spec = sp.get_spectrogram();
  // std::cout << spec.size() << ',' << spec[0].size() << '\n';
  for (auto i:lm) std::cout << i.time << ',' << i.hertz << std::endl;
  return 0;
}
