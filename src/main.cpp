#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <iostream>
#include <ranges>
#include <spdlog/spdlog.h>

auto main() -> int {

  Audio<double> audio("assets/3. You & Me - Good & Evil.wav");

  Spectrogram spectrogram(audio);

  auto spec = spectrogram.get_spectrogram();

  double sum{0};
  auto join_view{
      std::ranges::join_view(spec)}; // flatten mtx into a list of ints
  std::ranges::for_each(join_view, [&sum](auto n) { sum += n; });

  std::cout << sum << '\n';
}
