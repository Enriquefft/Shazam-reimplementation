#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <csvdumps.hpp>
#include <chrono>



auto main() -> int {

  std::vector<std::filesystem::path> songs = {"assets/the_bidding.wav",
                                                "assets/the_bidding_noisy_sample.wav" };

  using TypeParam = double;

  for (const auto &song : songs) {

    print("Hashing {}", song.string());
    print("Reading audio...");
    auto start = std::chrono::high_resolution_clock::now();
    Audio<TypeParam> audio(song);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now() - start).count();
    print("Read audio in {} ms",ms);
    
    print("Building Spectrogram");
    start = std::chrono::high_resolution_clock::now();
    Spectrogram<TypeParam> spectrogram(audio);
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now() - start).count();
    print("Built spectrogram in {} ms",ms);

    print("Extracting features...");
    start = std::chrono::high_resolution_clock::now();
    auto lm = spectrogram.get_local_maximums();
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now() - start).count();
    print("Found {} features in {} ms",lm.size(),ms);

    print("Computing hashes...");
    start = std::chrono::high_resolution_clock::now();
    auto hashes = spectrogram.get_hashes();
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now() - start).count();
    print("Computed {} hashes in {} ms",hashes.size(),ms);

    print("Dumping hashes");
    auto hashpath = song.parent_path().parent_path() / "experiments" / (song.stem().string() + "_hashes.csv");
    csvWriteHashes(hashes,hashpath);
    print("Hashes dumped at {}", hashpath.string());


    // auto complex_spectrogram = Spectrogram<TypeParam>::stft(audio);

    // std::complex<TypeParam> comp_sum = sum_vector(complex_spectrogram);

    // print("complex_spectrogram_sum: {}, {}", comp_sum.real(), comp_sum.imag());
    // print("complex_spectrogram_shape: {}, {}", complex_spectrogram.size(),
    //     complex_spectrogram.at(0).size());

    // spectrogram.get_local_maximums();
    // auto local_max = spectrogram.get_local_maximums();
    // auto spec = spectrogram.get_spectrogram();

    // auto sppath = song.parent_path().parent_path() / "experiments" / (song.stem().string() + "_spec.csv");
    // auto ptspath = song.parent_path().parent_path() / "experiments" / (song.stem().string() + "_crits.csv");
    // print("Dumping spectrogram... @{}", sppath.string());
    // csvWriteSpectrogram(spectrogram,sppath);
    // csvWriteLocalMaxima(local_max,ptspath);
    // db.insert(sp.get_features)
  }

  // db.search(ne_song)

  return 0;
}
