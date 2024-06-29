#ifndef INCLUDE_SPOTIFY_HPP_
#define INCLUDE_SPOTIFY_HPP_

#include <string>
#include <vector>

/**
 * @brief The Spotify api class
 * @details This class is used to interact with the Spotify API
 */
class Spotify {
private:
  std::string m_access_token;
  std::string m_token_type;
  int m_expires_in;

public:
  /// @brief Constructor
  /// @param client_id The Spotify client ID
  /// @param client_secret The Spotify client secret
  /// @details Initializes the Spotify class with the given client ID and client
  Spotify(const std::string &client_id, const std::string &client_secret);

  /// @brief Default constructor
  /// @details Initializes the Spotify class using the SPOTIFY_CLIENT_ID and
  /// SPOTIFY_CLIENT_SECRET environment variables
  Spotify();

  /// @brief get a list of track names from a playlist
  /// @param playlist_id The Spotify playlist ID
  /// @return A list of track names
  /// @details This function fetches the track names from a Spotify playlist
  [[nodiscard]] auto get_track_names(const std::string &playlist_id) const
      -> std::vector<std::string>;
};

#endif // INCLUDE_SPOTIFY_HPP_
