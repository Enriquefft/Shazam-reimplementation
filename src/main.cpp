#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <chrono>
#include <csvdumps.hpp>
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <fstream>

using std::cout;
namespace fs = std::filesystem;
using TypeParam = double;

/// @brief Checks if a path is a directory and optionally creates it if it doesn't exist
/// @param path The path to check/create
/// @param create_if_not_exist Flag indicating whether to create the directory if it doesn't exist
void check_directory(const fs::path& path, bool create_if_not_exist = false)
{
    if (fs::exists(path)) {
        if (!fs::is_directory(path)) {
            spdlog::critical("The path '{}' exists but is not a directory.", path.string());
            throw std::runtime_error("The path exists but is not a directory.");
        }
    } else {
        if (create_if_not_exist) {
            if (!fs::create_directory(path)) {
                spdlog::critical("Failed to create the directory '{}'.", path.string());
                throw std::runtime_error("Failed to create the directory.");
            }
        } else {
            spdlog::critical("The directory '{}' does not exist.", path.string());
            throw std::runtime_error("The directory does not exist.");
        }
    }
}

std::ofstream open_file(const fs::path& hashpath, const std::string& filename)
{
    std::ofstream file(hashpath / filename);
    if (!file) {
        spdlog::critical("Failed to open the file '{}'.", (hashpath / filename).string());
        throw std::runtime_error("Failed to open the file: " + filename);
    }
    return file;
}

void hash_songs(fs::path assets, fs::path hashpath)
{
    // Check both paths are okay
    check_directory(assets);
    check_directory(hashpath, true);

    // Open 'hashes.csv' and 'songs.csv' as ofstreams in hashpath
    auto hashes_file = open_file(hashpath, "hashes.csv");
    auto songs_file = open_file(hashpath, "songs.csv");

    // Stopwatch for timing
    spdlog::stopwatch sw_total; // Total time stopwatch

    // Hash all songs in assets
    size_t songid = 0;
    for (const auto& entry : fs::directory_iterator(assets)) {
        if (entry.is_regular_file()) {
            const auto& path = entry.path();
            const auto& fname = path.filename();
            if (path.extension() != ".wav") {
                spdlog::info("Omitting non-wav file {}", fname.string());
                continue;
            }

            // Stopwatch for per song timing
            spdlog::stopwatch sw_song; // Per song stopwatch

            // Step 1: Read song
            spdlog::stopwatch sw_step1; // Stopwatch for step 1
            Audio<TypeParam> song(path.string());
            spdlog::debug("Step 1 (Read song) took {} ms", sw_step1.elapsed().count());
            

            // Step 2: Create spectrogram
            spdlog::stopwatch sw_step2; // Stopwatch for step 2
            Spectrogram<TypeParam> spec(song);
            spdlog::debug("Step 2 (Create spectrogram) took {} ms", sw_step2.elapsed().count());
            

            // Step 3: Find local maxima
            spdlog::stopwatch sw_step3; // Stopwatch for step 3
            spdlog::debug("Step 3 (Find local maxima) took {} ms", sw_step3.elapsed().count());
            spec.get_local_maximums();
            

            // Step 4: Generate hashes
            spdlog::stopwatch sw_step4; // Stopwatch for step 4
            auto hashes = spec.get_hashes();
            spdlog::debug("Step 4 (Generate hashes) took {} ms", sw_step4.elapsed().count());
            

            // Step 5: Dump to files
            spdlog::stopwatch sw_step5; // Stopwatch for step 5
            songs_file << songid << ',' << fname << '\n';
            for (const auto& i : hashes) {
                uint32_t hash = i.first;
                size_t time = i.second;
                hashes_file << hash << ',' << time << ',' << songid << '\n';
            }
            spdlog::debug("Step 5 (Dump to files) took {} ms", sw_step5.elapsed().count());

            
            spdlog::info("Processed {} in {} ms", fname.string(), sw_song.elapsed().count());

            ++songid;
        }
    }

    // Close the files explicitly
    hashes_file.close();
    songs_file.close();

    // Log total time for hashing the entire assets folder
    spdlog::info("Total time to hash all songs: {} ms", sw_total.elapsed().count());
}

auto main() -> int {

  /*
  std::vector<std::filesystem::path> songs = {
      "assets/the_bidding.wav", "assets/the_bidding_noisy_sample.wav"};


  for (const auto &song : songs) {
    
    cout << "Reading audio..." << '\n';
    cout << "Hashing " << song.string() << '\n';

    auto start = std::chrono::high_resolution_clock::now();
    Audio<TypeParam> audio(song);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::high_resolution_clock::now() - start)
                  .count();

    cout << "Read audio in " << ms << " ms" << '\n';

    cout << "Building Spectrogram..." << '\n';

    start = std::chrono::high_resolution_clock::now();
    Spectrogram<TypeParam> spectrogram(audio);
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::high_resolution_clock::now() - start)
             .count();

    cout << "Built spectrogram in " << ms << " ms" << '\n';

    cout << "Extracting features..." << '\n';
    start = std::chrono::high_resolution_clock::now();
    auto lm = spectrogram.get_local_maximums();
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::high_resolution_clock::now() - start)
             .count();

    cout << "Found " << lm.size() << " features in " << ms << " ms" << '\n';

    cout << "Computing hashes..." << '\n';

    start = std::chrono::high_resolution_clock::now();
    auto hashes = spectrogram.get_hashes();
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::high_resolution_clock::now() - start)
             .count();
    cout << "Computed " << hashes.size() << " hashes in " << ms << " ms"
         << '\n';

    cout << "Dumping hashes..." << '\n';
    auto hashpath = song.parent_path().parent_path() / "experiments" /
                    (song.stem().string() + "_hashes.csv");
    csvWriteHashes(hashes, hashpath);
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
