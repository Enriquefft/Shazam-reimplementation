#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <csvdumps.hpp>



auto main() -> int {

  std::vector<std::filesystem::path> songs = {"assets/the_bidding.wav",
                                                "assets/the_bidding_noisy.wav" };

  using TypeParam = double;

  for (const auto &song : songs) {

    print("Hashing {}", song.string());
    Audio<TypeParam> audio(song);
    Spectrogram<TypeParam> spectrogram(audio);
    // auto complex_spectrogram = Spectrogram<TypeParam>::stft(audio);

    // std::complex<TypeParam> comp_sum = sum_vector(complex_spectrogram);

    // print("complex_spectrogram_sum: {}, {}", comp_sum.real(), comp_sum.imag());
    // print("complex_spectrogram_shape: {}, {}", complex_spectrogram.size(),
    //     complex_spectrogram.at(0).size());

    // spectrogram.get_local_maximums();
    auto local_max = spectrogram.get_local_maximums();
    // auto spec = spectrogram.get_spectrogram();

    auto sppath = song.parent_path().parent_path() / "experiments" / (song.stem().string() + "_spec.csv");
    auto ptspath = song.parent_path().parent_path() / "experiments" / (song.stem().string() + "_crits.csv");
    print("Dumping spectrogram... @{}", sppath.string());
    csvWriteSpectrogram(spectrogram,sppath);
    csvWriteLocalMaxima(local_max,ptspath);
    // db.insert(sp.get_features)
  }

  // db.search(ne_song)

  return 0;
}
