#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

#include <HashLoading.hpp>
#include <cstdlib>
#include <search.hpp>

using TypeParam = double;

auto main(int argc, char *argv[]) -> int {
  spdlog::set_level(spdlog::level::debug);
  if (argc < 4) {
    std::cout << "Missing arguments: you must call this program like \n "
                 " query <query song path> <hashes directory> <songs directory>"
              << '\n';
    return 0;
  }
  std::filesystem::path querysong_path(argv[1]);
  std::filesystem::path hashes_dir(argv[2]);
  std::filesystem::path songs_dir(argv[3]);

  auto hashes = load_song_hashes(hashes_dir / "hashes.csv");
  auto songids = load_song_ids(hashes_dir / "songs.csv");
  Audio<TypeParam> querysong(querysong_path);
  auto res = search_song<TypeParam>(hashes, songids, querysong);

  std::string command;
  command = "aplay \"";
  command += (songs_dir / res.value()).string();
  command += "\"";

  system(command.c_str());

  return 0;
}
