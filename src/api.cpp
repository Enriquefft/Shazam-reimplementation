#include "AudioFile.hpp"
#include "HashLoading.hpp"
#include "Spectrogram.hpp"
#include "search.hpp"
#include <crow/app.h>
#include <crow/common.h>
#include <crow/multipart.h>

using TypeParam = double;

constexpr uint16_t PORT = 443;

auto main() -> int {

  // muy xd
  auto hashes = load_song_hashes("experiments/hashes/hashes.csv");
  auto songids = load_song_ids("experiments/hashes/songs.csv");

  std::cout << "Ok1" << '\n';

  crow::SimpleApp app;

  CROW_ROUTE(app, "/").methods(
      "POST"_method)([&hashes, &songids](const crow::request &req) {
    crow::response res;

    // Get the content type from the request header
    std::string content_type = req.get_header_value("Content-Type");
    // Check if the content type is multipart/form-data
    if (content_type.find("multipart/form-data") == std::string::npos) {
      // Return a response to the client with an error message and a status code
      // of 400 Bad Request
      res.code = crow::BAD_REQUEST;
      res.write("Invalid content type"); // Change here
      res.end();
    }
    try {
      // Parse the request body using the crow::multipart::message class
      crow::multipart::message msg(req);
      std::cout << "Ok3" << '\n';

      auto messages = msg.part_map;

      auto audio = messages.find("audio_data")->second.body;

      auto audio_size = audio.size();
      const auto *audio_data = audio.c_str();

      try {
        std::ofstream out("temp_audio.wav", std::ios::binary);
        out.write(audio_data, static_cast<int64_t>(audio_size));
        out.close();
      } catch (const std::ios_base::failure &e) {
        // Catch any file operation failures and print the error
        std::cout << "File operation failed: " << e.what() << '\n';
      }

      // create & search the audio
      Audio<TypeParam> audio_object("temp_audio.wav");

      auto best_match = search_song<TypeParam>(hashes, songids, audio_object);

      if (!best_match) {
        res.code = crow::NOT_FOUND;
        res.write("No significant match found");
        res.end();
      } else {
        res.code = crow::OK;
        res.write(std::string{best_match->stem()});
        res.end();
      }

    } catch (const std::exception &e) {
      std::cout << "Exception occurred: " << e.what() << '\n';
      res.code = crow::INTERNAL_SERVER_ERROR;
      res.write("Internal server error");
      res.end();
    }
    return res;
  });

  CROW_ROUTE(app, "/health")([]() { return "Hello world"; });

  app.port(PORT).multithreaded().run();
}
