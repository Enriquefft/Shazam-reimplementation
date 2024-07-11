#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

#include <search.hpp>
#include <HashLoading.hpp>
#include <cstdlib>

using TypeParam = double;

auto main(int argc, char *argv[]) -> int {
  spdlog::set_level(spdlog::level::debug);
  if (argc < 4)
  {
    std::cout << "Missing arguments: you must call this program like \n "
    " query <query song path> <hashes directory> <songs directory>" << std::endl;
    return 0;
  }
  std::filesystem::path querysong_path(argv[1]);
  std::filesystem::path hashes_dir(argv[2]);
  std::filesystem::path songs_dir(argv[3]);



  auto hashes = load_song_hashes(hashes_dir / "hashes.csv");
  auto songids = load_song_ids(hashes_dir /"songs.csv");
  auto res = search<TypeParam>(hashes,songids,querysong_path);
  
  std::string command;
  command = "aplay \"";
  command += (songs_dir / res).string();
  command += "\"";
  
  system(command.c_str());
  

  return 0;
}
