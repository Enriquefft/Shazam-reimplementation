#include <HashDumping.hpp>
#include <filesystem>
#include <iostream>

using TypeParam = double;

auto main(int argc, char *argv[]) -> int {
  std::filesystem::path ASSETS_PATH;
  std::filesystem::path HASHES_PATH;
  if (argc < 3) {
    std::cout << "Missing arguments: you must call this program like \n "
                 " index <songs to index directory> <hash dump directory>"
              << std::endl;
    return 0;
  }
  ASSETS_PATH = std::filesystem::path(argv[1]);
  HASHES_PATH = std::filesystem::path(argv[2]);
  hash_songs<TypeParam>(ASSETS_PATH, HASHES_PATH);
}
