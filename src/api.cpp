#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include "HashLoading.hpp"
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

  CROW_ROUTE(app, "/").methods("POST"_method)([ &hashes, &songids ](const crow::request &req) {
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

      // create & search the audio
      Audio<TypeParam> audio_object(audio_data);
      // hay un oveload para esto en search.hpp!
      auto bestMatch = search<TypeParam>(hashes,songids,audio_object);
      // xd isimo xddd
      return bestMatch.string().c_str();

      try {
        std::ofstream out("outfiles/s1", std::ios::binary);
        out.write(audio_data, static_cast<int64_t>(audio_size));
        out.close();
      } catch (const std::ios_base::failure &e) {
        // Catch any file operation failures and print the error
        std::cout << "File operation failed: " << e.what() << '\n';
      }

      // Return a response to the client with a success message and a status
      // code of 200 OK
      res.code = crow::CREATED;
      res.write("File uploaded successfully"); // Change here
      res.end();
    } catch (const std::exception &e) {
      // Catch any general exceptions and print the error message
      std::cout << "Exception occurred: " << e.what() << '\n';
      // Return a response to the client with an error message and a status
      // code of 500 Internal Server Error
      res.code = crow::INTERNAL_SERVER_ERROR;
      res.write("Internal server error"); // Change here
      res.end();
    }

    return "Hello world";
  });

  CROW_ROUTE(app, "/health")([]() { return "Hello world"; });

  app.port(PORT).multithreaded().run();
}
