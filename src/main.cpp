#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <chrono>
#include <csvdumps.hpp>
#include <filesystem>
#include <iostream>

#include <SongIndexing.hpp>


using std::cout;
namespace fs = std::filesystem;
using TypeParam = double;

auto main() -> int {

  hash_songs<TypeParam>("./assets", "./experiments/hashes");
  /*
  std::vector<std::filesystem::path> songs = {
      "assets/the_bidding.wav", "assets/the_bidding_noisy_sample.wav"};


  for (const auto &song : songs) {
    
    cout << "Reading audio..." << '\n';
    cout << "Hashing " << song.string() << '\n';

    auto start = std::chrono::high_resolution_clock::now();
    Audio<TypeParam> audio(song);
    auto end = std::chrono::high_resolution_clock::now();
    auto m_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - start)
                         .count();

    cout << "Read audio in " << m_seconds << " ms" << '\n';

    cout << "Building Spectrogram..." << '\n';

    start = std::chrono::high_resolution_clock::now();
    Spectrogram<TypeParam> spectrogram(audio);
    end = std::chrono::high_resolution_clock::now();
    m_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start)
                    .count();

    cout << "Built spectrogram in " << m_seconds << " ms" << '\n';

    cout << "Extracting features..." << '\n';
    start = std::chrono::high_resolution_clock::now();
    auto local_max = spectrogram.get_local_maximums();
    end = std::chrono::high_resolution_clock::now();
    m_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start)
                    .count();

    cout << "Found " << local_max.size() << " features in " << m_seconds
         << " ms" << '\n';

    cout << "Computing hashes..." << '\n';

    start = std::chrono::high_resolution_clock::now();
    auto hashes = spectrogram.get_hashes();
    end = std::chrono::high_resolution_clock::now();
    m_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - start)
                    .count();
    cout << "Computed " << hashes.size() << " hashes in " << m_seconds << " ms"
         << '\n';

    cout << "Dumping hashes..." << '\n';
    auto hashpath = song.parent_path().parent_path() / "experiments" /
                    (song.stem().string() + "_hashes.csv");
    csv_write_hashes(hashes, hashpath);
    cout << "Hashes dumped at " << hashpath.string() << '\n';

    // auto complex_spectrogram = Spectrogram<TypeParam>::stft(audio);

    // std::complex<TypeParam> comp_sum = sum_vector(complex_spectrogram);

    // print("complex_spectrogram_sum: {}, {}", comp_sum.real(),
    // comp_sum.imag()); print("complex_spectrogram_shape: {}, {}",
    // complex_spectrogram.size(),
    //     complex_spectrogram.at(0).size());

    // spectrogram.get_local_maximums();
    // auto local_max = spectrogram.get_local_maximums();
    // auto spec = spectrogram.get_spectrogram();

    // auto sppath = song.parent_path().parent_path() / "experiments" /
    // (song.stem().string() + "_spec.csv"); auto ptspath =
    // song.parent_path().parent_path() / "experiments" / (song.stem().string()
    // + "_crits.csv"); print("Dumping spectrogram... @{}", sppath.string());
    // csvWriteSpectrogram(spectrogram,sppath);
    // csvWriteLocalMaxima(local_max,ptspath);
    // db.insert(sp.get_features)
  }

  // db.search(ne_song)
  */

  return 0;
}
