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

  crow::SimpleApp app;

  CROW_ROUTE(app, "/").methods(
      "POST"_method)([&hashes, &songids](const crow::request &req) {
    crow::response response;
    crow::json::wvalue response_body;

    // Get the content type from the request header
    std::string content_type = req.get_header_value("Content-Type");
    // Check if the content type is multipart/form-data
    if (content_type.find("multipart/form-data") == std::string::npos) {
      // Return a response to the client with an error message and a status code
      // of 400 Bad Request
      response.code = crow::BAD_REQUEST;
      response_body["error"] = "Invalid content type";
    }
    try {
      // Parse the request body using the crow::multipart::message class
      crow::multipart::message msg(req);

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
        response.code = crow::NOT_FOUND;
        response_body["error"] = "No significant match found";
        std::cout << "No significant match found" << '\n';

      } else {
        response.code = crow::OK;
        response_body["song_name"] = best_match->stem().string();
        std::cout << "Best match found: " << best_match->stem().string()
                  << '\n';
      }

    } catch (const std::exception &e) {
      std::cout << "Exception occurred: " << e.what() << '\n';
      response.code = crow::INTERNAL_SERVER_ERROR;
      response_body["error"] = "Internal server error";
    }
    response.write(response_body.dump());
    return response;
  });

  CROW_ROUTE(app, "/health")([]() { return "Hello world"; });

  app.port(PORT).multithreaded().run();
}
