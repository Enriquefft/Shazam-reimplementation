#include <AudioFile.hpp>
#include <Spectrogram.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

auto main() -> int {
  Audio<float> a("assets/the_bidding.wav");
  Spectrogram<float> sp(a);
  sp.get_local_maximums();
  std::cout << "Local Maxima found (processed " << sp.get_x() << ','
            << sp.get_y() << ") pixels" << '\n';
  return 0;
}
