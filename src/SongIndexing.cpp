#include <HashDumping.hpp>
#include <filesystem>
#include <iostream>
#include <ParseConfig.hpp>

namespace fs = std::filesystem;
using std::string;

using TypeParam = double;

auto main(int argc, char *argv[]) -> int {
  fs::path songs;
  fs::path hashdump;
  fs::path datadump;
  fs::path configfile;
  Config configuration;

  bool dumpWasGiven = false;
  bool configFileGiven = false;

  // mandatory args
  if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " <path/to/songs/folder> <path/to/hashes/dump> [--config path/to/config.ini] [--dump path/to/dump/directory]" << std::endl;
      return 1;
  }
  songs = argv[1];
  hashdump = argv[2];

  // optional args
  for (int i=3;i<argc;i++)
  {
    string arg = argv[i];
    if (arg == "--config" && i + 1 < argc)
    {
      configfile = argv[++i];
    }
    else if (arg == "--dump" && i + 1 < argc)
    {
      dumpWasGiven = true;
      datadump = argv[++i];
    }
  }
  
  // read the config file
  if (configFileGiven)
    Config configuration = parseConfig(configfile);

  if (dumpWasGiven)
  {

    std::cout << "A . will be drawn for each song hashed." << std::endl;
    hash_songs_and_dump_stats<TypeParam>(
            songs,
            hashdump,
            datadump,
            configuration
    );
  }
  else
  {
    hash_songs<TypeParam>(songs,hashdump,configuration);
  }
}
