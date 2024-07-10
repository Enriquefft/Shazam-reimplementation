#include "AudioFile.hpp"
#include "UnorderedMap.hpp"
#include <algorithm>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

using TypeParam = double;

using hash_t = std::pair<uint32_t, size_t>;

constexpr auto count_matches(const std::vector<hash_t> &song_hash,
                             const std::vector<hash_t> &hashes) -> size_t {
  size_t match_count = 0;

  std::ranges::for_each(song_hash, [&](const auto &hash_frame) {
    std::ranges::for_each(hashes, [&](const auto &hash) {
      if (hash_frame == hash) {
        match_count++;
      }
    });
  });

  return match_count;
}

auto main() -> int {

  spdlog::set_level(spdlog::level::debug);
  spdlog::stopwatch sw_total; // Total time stopwatch

  const std::string HASHES_FILE = "songs_map.bin";
  const std::filesystem::path SONG_TO_SEARCH = "assets/banana_man_sample.wav";

  std::cout << "Searching for " << SONG_TO_SEARCH << " in the database\n";

  spdlog::stopwatch sw_step1;
  auto songs_map = read_songs_map_from_file<TypeParam>(HASHES_FILE);
  spdlog::debug("Step 1 (Read songs_map) took {} seconds",
                sw_step1.elapsed().count());

  spdlog::stopwatch sw_step2;
  Audio<TypeParam> song(SONG_TO_SEARCH);
  spdlog::debug("Step 2 generate song audiodata took {} seconds",
                sw_step2.elapsed().count());

  spdlog::stopwatch sw_step3;
  Spectrogram<TypeParam> spectrogram(song);
  spdlog::debug("Step 3 generate song spectrogram took {} seconds",
                sw_step3.elapsed().count());

  spdlog::stopwatch sw_step4;
  spectrogram.get_local_maximums();
  auto song_hash = spectrogram.get_hashes();
  spdlog::debug("Step 4 generate song hashes took {} seconds",
                sw_step4.elapsed().count());

  std::unordered_map<std::string, size_t> matches;

  spdlog::stopwatch sw_step5;
  for (const auto &[song_name, song_hashes] : songs_map) {
    matches[song_name] = count_matches(song_hash, song_hashes);
  }
  spdlog::debug(
      "Step 5 compare hash to every song in songs_map took {} seconds",
      sw_step5.elapsed().count());

  for (const auto &[song_name, count] : matches) {
    std::cout << song_name << " : " << count << '\n';
  }

  spdlog::info("Total time to search for {} in songs_index: {} ms",

               std::string{SONG_TO_SEARCH.stem()}, sw_total.elapsed().count());

  return 0;
}
