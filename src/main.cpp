#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <filesystem>
#include <spdlog/spdlog.h>

auto main() -> int {

  std::vector<std::filesystem::path> songs = {"assets/the_bidding.wav"};

  using TypeParam = double;

  for (const auto &song : songs) {

    Audio<TypeParam> audio(song);
    Spectrogram spectrogram(audio);
    auto complex_spectrogram = Spectrogram<TypeParam>::stft(audio);

    print("audiodata_sum: {}", sum_vector(audio.m_audiodata));
    print("audiodata_shape: {}", audio.m_audiodata.size());

    std::complex<TypeParam> comp_sum = sum_vector(complex_spectrogram);

    print("complex_spectrogram_sum: {}, {}", comp_sum.real(), comp_sum.imag());
    print("complex_spectrogram_shape: {}", complex_spectrogram.size());

    spectrogram.get_local_maximums();
    auto local_max = spectrogram.get_local_maximums();
    auto spec = spectrogram.get_spectrogram();
    // for (const auto &max : local_max) {
    // std::cout << max.time << ',' << max.hertz << '\n';
    // }

    // db.insert(sp.get_features)
  }

  // db.search(ne_song)

  return 0;
}
