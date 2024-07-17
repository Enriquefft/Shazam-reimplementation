#ifndef INCLUDE_HASHDUMPING_HPP_
#define INCLUDE_HASHDUMPING_HPP_

#include "HashDumping.hpp"
#include <csvdumps.hpp>
#include <AudioFile.hpp>
#include <Spectrogram.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <unordered_map>

namespace fs = std::filesystem;
using TypeParam = double;

/// @brief Checks if a path is a directory and optionally creates it if it
/// doesn't exist
/// @param path The path to check/create
/// @param create_if_not_exist Flag indicating whether to create the directory
/// if it doesn't exist
inline void check_directory(const fs::path &path,
                            bool create_if_not_exist = false) {
  if (fs::exists(path)) {
    if (!fs::is_directory(path)) {
      std::cerr << "The path '" << path.string() << "' exists but is not a directory." << std::endl;
      throw std::runtime_error("The path exists but is not a directory.");
    }
  } else {
    if (create_if_not_exist) {
      if (!fs::create_directory(path)) {
        std::cerr << "Failed to create the directory '" << path.string() << "'." << std::endl;
        throw std::runtime_error("Failed to create the directory.");
      }
    } else {
      std::cerr << "The directory '" << path.string() << "' does not exist." << std::endl;
      throw std::runtime_error("The directory does not exist.");
    }
  }
}

inline auto open_file(const fs::path &hashpath, const std::string &filename)
    -> std::ofstream {
  std::ofstream file(hashpath / filename);
  if (!file) {
    std::cerr << "Failed to open the file '" << (hashpath / filename).string() << "'." << std::endl;
    throw std::runtime_error("Failed to open the file: " + filename);
  }
  return file;
}

/// @brief Hashes all songs in the assets folder and writes them to an unordered
/// map.
template <std::floating_point T>
auto hash_songs(const fs::path &assets , const fs::path &hashpath) {
  // Check both paths are okay
  check_directory(assets);
  check_directory(hashpath, true);

  // Open 'hashes.csv' and 'songs.csv' as ofstreams in hashpath
  auto hashes_file = open_file(hashpath, "hashes.csv");
  auto songs_file = open_file(hashpath, "songs.csv");

  // Stopwatch for timing
  auto start_total = std::chrono::high_resolution_clock::now();

  // Hash all songs in assets
  for (size_t songid = 0; const auto &entry : fs::directory_iterator(assets)) {

    if (!entry.is_regular_file()) {
      continue;
    }

    const auto &path = entry.path();
    const auto &fname = path.filename();
    if (path.extension() != ".wav") {
      std::cout << "Omitting non-wav file " << fname.string() << std::endl;
      continue;
    }
    std::cout << "Hashing " << fname.string() << std::endl;

    // Stopwatch for per song timing
    auto start_song = std::chrono::high_resolution_clock::now();

    // Step 1: Read song
    auto start_step1 = std::chrono::high_resolution_clock::now();
    Audio<T> song(path.string());
    auto end_step1 = std::chrono::high_resolution_clock::now();
    std::cout << "Read song took " 
              << std::chrono::duration_cast<std::chrono::seconds>(end_step1 - start_step1).count() 
              << " seconds" << std::endl;

    // Step 2: Create spectrogram
    auto start_step2 = std::chrono::high_resolution_clock::now();
    Spectrogram<T> spec(song);
    auto end_step2 = std::chrono::high_resolution_clock::now();
    std::cout << "Create spectrogram took " 
              << std::chrono::duration_cast<std::chrono::seconds>(end_step2 - start_step2).count() 
              << " seconds" << std::endl;

    // Step 3: Find local maxima
    auto start_step3 = std::chrono::high_resolution_clock::now();
    spec.get_local_maximums();
    auto end_step3 = std::chrono::high_resolution_clock::now();
    std::cout << "Find local maxima took " 
              << std::chrono::duration_cast<std::chrono::seconds>(end_step3 - start_step3).count() 
              << " seconds. Found " << spec.get_feature_count() << " local maxima." << std::endl;

    // Step 4: Generate hashes
    auto start_step4 = std::chrono::high_resolution_clock::now();
    auto hashes = spec.get_hashes();
    auto end_step4 = std::chrono::high_resolution_clock::now();
    std::cout << "Generate hashes took " 
              << std::chrono::duration_cast<std::chrono::seconds>(end_step4 - start_step4).count() 
              << " seconds. Generates " << hashes.size() << " hashes" << std::endl;

    // Step 5: Dump to files
    auto start_step5 = std::chrono::high_resolution_clock::now();
    songs_file << songid << ',' << fname << '\n';
    for (const auto &i : hashes) {
      uint32_t hash = i.first;
      size_t time = i.second;
      hashes_file << hash << ',' << time << ',' << songid << '\n';
    }
    auto end_step5 = std::chrono::high_resolution_clock::now();
    std::cout << "Step 5 (Dump to files) took " 
              << std::chrono::duration_cast<std::chrono::seconds>(end_step5 - start_step5).count() 
              << " seconds" << std::endl;

    ++songid;
  }

  // Close the files explicitly
  hashes_file.close();
  songs_file.close();

  // Log total time for hashing the entire assets folder
  auto end_total = std::chrono::high_resolution_clock::now();
  std::cout << "Total time to hash all songs: " 
            << std::chrono::duration_cast<std::chrono::milliseconds>(end_total - start_total).count() 
            << " ms" << std::endl;
  return;
}

#endif // INCLUDE_HASHDUMPING_HPP_
