#ifndef INCLUDE_UNORDEREDMAP_HPP_
#define INCLUDE_UNORDEREDMAP_HPP_

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

#include "Spectrogram.hpp"
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility> // for std::pair

template <std::floating_point T>
using hashes_t = decltype(std::declval<Spectrogram<T>>().get_hashes());

template <std::floating_point T>
void write_songs_map_to_file(
    const std::unordered_map<std::string, hashes_t<T>> &songs_map,
    const std::string &filename) {
  std::ofstream out(filename, std::ios::binary);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open file for writing");
  }

  size_t map_size = songs_map.size();
  out.write(reinterpret_cast<const char *>(&map_size), sizeof(map_size));

  for (const auto &[key, value] : songs_map) {
    size_t key_size = key.size();
    out.write(reinterpret_cast<const char *>(&key_size), sizeof(key_size));
    out.write(key.data(), static_cast<int64_t>(key_size));

    size_t vec_size = value.size();
    out.write(reinterpret_cast<const char *>(&vec_size), sizeof(vec_size));
    out.write(reinterpret_cast<const char *>(value.data()),
              vec_size * sizeof(typename hashes_t<T>::value_type));
  }
}

template <std::floating_point T>
auto read_songs_map_from_file(const std::string &filename)
    -> std::unordered_map<std::string, hashes_t<T>> {
  std::ifstream input_file(filename, std::ios::binary);
  if (!input_file.is_open()) {
    throw std::runtime_error("Failed to open file for reading");
  }

  std::unordered_map<std::string, hashes_t<T>> songs_map;
  size_t map_size = 0;
  input_file.read(reinterpret_cast<char *>(&map_size), sizeof(map_size));

  for (size_t i = 0; i < map_size; ++i) {
    size_t key_size = 0;
    input_file.read(reinterpret_cast<char *>(&key_size), sizeof(key_size));

    std::string key(key_size, '\0');
    input_file.read(key.data(), static_cast<int64_t>(key_size));

    size_t vec_size = 0;
    input_file.read(reinterpret_cast<char *>(&vec_size), sizeof(vec_size));

    hashes_t<T> value(vec_size);
    input_file.read(reinterpret_cast<char *>(value.data()),
                    vec_size * sizeof(typename hashes_t<T>::value_type));

    songs_map.emplace(std::move(key), std::move(value));
  }

  return songs_map;
}

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

#endif // INCLUDE_UNORDEREDMAP_HPP_
