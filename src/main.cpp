#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <iostream>
#include <ranges>
#include <spdlog/spdlog.h>

auto main() -> int {

  Audio<double> audio("assets/3. You & Me - Good & Evil.wav");

  Spectrogram spectrogram(audio);

  auto spec = spectrogram.get_spectrogram();
}
