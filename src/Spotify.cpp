#include "Spotify.hpp"
#include <cpr/api.h>
#include <cstdlib>
#include <format>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string_view>

using json = nlohmann::json;

constexpr std::string_view SPOTIFY_BASE_URL = "https://api.spotify.com/v1";
constexpr std::string_view SPOTIFY_TOKEN_URL =
    "https://accounts.spotify.com/api/token";
constexpr auto SPOTIFY_RESPONSE_LIMIT = 50;

Spotify::Spotify(const std::string &client_id,
                 const std::string &client_secret) {

  std::cout << "client_id: " << client_id << '\n';
  std::cout << "client_secret: " << client_secret << '\n';

  cpr::Response response = cpr::Post(
      cpr::Url{SPOTIFY_TOKEN_URL}, cpr::Payload{{"key", "value"}},
      cpr::Payload{{"grant_type", "client_credentials"},
                   {"client_id", client_id},
                   {"client_secret", client_secret}},
      cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"}});
  auto json_string = response.text;

  std::cout << "Json response:\t" << json_string << '\n';

  auto json_object = json::parse(json_string);

  m_access_token = json_object["access_token"];
  m_token_type = json_object["token_type"];
  m_expires_in = json_object["expires_in"];
}

inline auto get_env(const char *env_name) -> std::string {
  auto *env = std::getenv(env_name);
  if (env == nullptr) {
    throw std::runtime_error(
        std::format("Undefined environment variable: {}", env_name));
  }
  return env;
}

Spotify::Spotify()
    : Spotify(get_env("SPOTIFY_CLIENT_ID"), get_env("SPOTIFY_CLIENT_SECRET")) {}

auto Spotify::get_track_names(const std::string &playlist_id) const
    -> std::vector<std::string> {
  std::cout << playlist_id;
  // Fetch playlist tracks
  std::string playlist_url =
      std::string(SPOTIFY_BASE_URL) + "/playlists/" + playlist_id +
      std::format("/tracks?fields=items%28track%28name%29%29&limit={}",
                  SPOTIFY_RESPONSE_LIMIT);

  cpr::Response playlist_data =
      cpr::Get(cpr::Url{playlist_url},
               cpr::Header{{"Authorization", "Bearer " + m_access_token}});

  // Parse JSON response to extract track names
  nlohmann::json playlist_json = nlohmann::json::parse(playlist_data.text);

  std::vector<std::string> track_names;
  track_names.reserve(SPOTIFY_RESPONSE_LIMIT);

  for (const auto &item : playlist_json["items"]) {
    track_names.push_back(item["track"]["name"]);
  }

  return track_names;
}
