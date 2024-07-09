#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <filesystem>
#include <concepts>
#include <fstream>
#include <Spectrogram.hpp>
#include <AudioFile.hpp>

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

template<std::floating_point T>
void hash_songs(fs::path assets, fs::path hashpath)
{
    // Check both paths are okay
    check_directory(assets);
    check_directory(hashpath, true);

    // Open 'hashes.csv' and 'songs.csv' as ofstreams in hashpath
    auto hashes_file = open_file(hashpath, "hashes.csv");
    auto songs_file = open_file(hashpath, "songs.csv");

    // Stopwatch for timing
    spdlog::set_level(spdlog::level::debug);
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
            spdlog::info("Hashing {}",fname.string());
            // Stopwatch for per song timing
            spdlog::stopwatch sw_song; // Per song stopwatch

            // Step 1: Read song
            spdlog::stopwatch sw_step1; // Stopwatch for step 1
            Audio<T> song(path.string());
            spdlog::debug("Step 1 (Read song) took {} ms", sw_step1.elapsed().count());
            

            // Step 2: Create spectrogram
            spdlog::stopwatch sw_step2; // Stopwatch for step 2
            Spectrogram<T> spec(song);
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