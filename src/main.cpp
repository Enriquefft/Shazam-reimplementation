#include <iostream>
#include <filesystem>
#include <HashLoading.hpp>
#include <search.hpp>
#include <string>
#include <algorithm>
#include "ParseConfig.hpp"
#include <HashDumping.hpp>
#include <chrono>

namespace fs = std::filesystem;
using TypeParam = double;

int main(int argc, char* argv[]) {
    fs::path hashes_path;
    bool is_sampleset = false;
    fs::path path_to_query;
    bool received_config = false;
    fs::path config_file;
    bool received_dump = false;
    fs::path dump_file;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " hashes/database/path [--sampleset] path/to/wav/or/directory/of/wavs [--config path/to/config.ini] [--dump path/to/dump/directory]" << std::endl;
        return 1;
    }

    hashes_path = argv[1];

    // Parse optional arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--sampleset" && i + 1 < argc) {
            is_sampleset = true;
            path_to_query = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            received_config = true;
            config_file = argv[++i];
        } else if (arg == "--dump" && i + 1 < argc) {
            received_dump = true;
            dump_file = argv[++i];
        } else {
            if (!is_sampleset) {
                path_to_query = argv[i];
            } else {
                std::cerr << "Unknown argument: " << arg << std::endl;
                return 1;
            }
        }
    }

    // Initialize Config object
    Config config;
    if (received_config) {
        config = parseConfig(config_file.string());
    }

    auto hashes = load_song_hashes(hashes_path / "hashes.csv");
    auto songids = load_song_ids(hashes_path / "songs.csv");

    if (!is_sampleset)
    {
      if (!fs::is_regular_file(path_to_query) || path_to_query.extension() != ".wav")
      {
        std::cerr << "File " << path_to_query << 
            "does not exist or is invalid" << std::endl;
        return 1;
      }
      Audio<TypeParam> sample(path_to_query);
      auto resp = search_song(hashes,songids,sample);
      if (resp.has_value())
      {
        std::cout << resp.value() << std::endl;
      }
      else
      {
        std::cout << "No viable match found";
      }
    }
    else
    {
      if (!fs::is_directory(path_to_query))
      {
        std::cerr << "Directory " << path_to_query << 
            "does not exist or is invalid" << std::endl;
        return 1;
      }

      std::ofstream dump;
      if (!received_dump)
      {
        std::cerr << "this type of query requires a dump location wich was not provided" << std::endl;
        return 1;
      }
      dump = open_file(dump_file,"scores.csv");


      // for each file in the directory
      for (auto& entry:fs::directory_iterator(path_to_query))
      {
        auto fpath = entry.path();
        if (!fs::is_regular_file(entry) || fpath.extension() != ".wav")
        {
          continue; // skip file
        }
        
        auto start = std::chrono::high_resolution_clock::now();

        Audio<TypeParam> sample(fpath);
        auto scores = score_songs(hashes,songids,sample,config);
        // sort by song id for canocical form
        std::sort(scores.begin(),scores.end(),
        [](std::pair<size_t,size_t> a,std::pair<size_t,size_t> b)
        {
          return a.second <= b.second;
        });
        
        auto end = std::chrono::high_resolution_clock::now();
        int elapsedms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        dump << fpath.filename() << ',' << elapsedms;

        // yes yes this is hacky... It depends on the way that songids get generated
        // basically iterate through the song ids, give 0 if it doesnt match and score
        // otherwise

        int j = 0;
        // scores vector for every song against this sample
        for (int i=0; i<songids.size();i++)
        {
          if (j < scores.size() && scores[j].second == i)
          {
            dump << ',' << scores[j].first;
            j++;
          }
          else
          {
            // no matches!
            dump << ',' << 0;
          }
        }
        dump << '\n';
      }
    }

    return 0;
}


/*
#include <HashLoading.hpp>
#include <cstdlib>
#include <search.hpp>

using TypeParam = double;

auto main(int argc, char *argv[]) -> int {
  

  // spdlog::set_level(spdlog::level::debug);
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

  if (!res.has_value()) {
    std::cout << "No song found" << '\n';
    return 0;
  }

  std::string command;
  command = "aplay \"";
  command += (songs_dir / res.value()).string();
  command += "\"";

  system(command.c_str());

  return 0;
}
*/