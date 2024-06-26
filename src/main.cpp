#include <Texture.hpp>
#include <Window.hpp>
#include <raylib.h>

#if defined(_WIN32)
#define NOGDI  // All GDI defines and routines
#define NOUSER // All USER defines and routines
#endif

#include <spdlog/spdlog.h>

#if defined(_WIN32) // raylib uses these names as function parameters
#undef near
#undef far
#endif

constexpr int TARGET_FPS = 60;

auto main() -> int {

  constexpr int SCREEN_WIDTH = 800;
  constexpr int SCREEN_HEIGHT = 450;

  constexpr int TEXT_X_POS = 190;
  constexpr int TEXT_Y_POS = 200;
  constexpr int TEXT_FONT_SIZE = 20;

  raylib::Window window(SCREEN_WIDTH, SCREEN_HEIGHT,
                        "raylib-cpp - basic window");
  raylib::Texture logo(ASSETS_PATH "dogo.png");

  SetTargetFPS(TARGET_FPS);

  spdlog::info("Started drawing");

  while (!window.ShouldClose()) {
    BeginDrawing();

    window.ClearBackground(RAYWHITE);

    // Object methods.
    logo.Draw(SCREEN_WIDTH / 2 - logo.GetWidth() / 2,
              SCREEN_HEIGHT / 2 - logo.GetHeight() / 2);

    DrawText("Congrats! You created your first window!", TEXT_X_POS, TEXT_Y_POS,
             TEXT_FONT_SIZE, LIGHTGRAY);

    EndDrawing();
  }
  spdlog::info("Finished drawing");

  return 0;
}
