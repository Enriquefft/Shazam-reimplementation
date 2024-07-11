cmake_minimum_required(VERSION 3.16)

project(
  SHAZAM_CPP
  VERSION 1.0
  DESCRIPTION "A reimplementation of Shazam"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CXX_STANDARD_REQUIRED on)

if(${CMAKE_VERSION} VERSION_LESS "3.25.2")
  message(
    STATUS
      "CMake version is less than 3.25.2, upgrade to silence unnecessary warnings"
  )
endif()

# ##############################################################################
# Targets

set(PROJECT_APP ${PROJECT_NAME}_APP)
set(PROJECT_APP_INDEXING ${PROJECT_NAME}_APP_INDEXING)
set(PROJECT_API ${PROJECT_NAME}_API)

add_library(${PROJECT_NAME} #[[ src/Spotify.cpp ]]
            src/AudioFile.cpp src/Spectrogram.cpp src/Features.cpp)

add_executable(${PROJECT_APP} src/main.cpp)
add_executable(${PROJECT_APP_INDEXING} src/SongIndexing.cpp)
add_executable(${PROJECT_API} src/api.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include/)

# ##############################################################################

add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_20)

set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION
                                                 TRUE CXX_STANDARD 20)

target_compile_options(
  common
  INTERFACE # Preferred warnings
            $<$<CXX_COMPILER_ID:MSVC>:
            /Wall
            >
            $<$<CXX_COMPILER_ID:Clang>:-Weverything>
            $<$<CXX_COMPILER_ID:GCC>:-fconcepts-diagnostics-depth=3>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
            # -Werror
            -Wall
            -Wextra
            -Wpedantic
            -Wformat=2
            -Wshadow
            -Wwrite-strings
            -Wcast-qual
            -Wcast-align
            -Wno-switch
            -Wconversion
            -Wno-ignored-optimization-argument
            > # Disable some warnings when using clang's Weverything
            $<$<CXX_COMPILER_ID:Clang>:
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++20-compat
            -Wno-c++20-extensions
            -Wno-c99-extensions
            -Wno-zero-as-null-pointer-constant
            -Wno-padded
            -Wno-global-constructors
            -Wno-exit-time-destructors
            # Temporarily removed due to bug upstream
            # (https://github.com/llvm/llvm-project/issues/59572)
            -Wno-unused-macros
            >
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined> #
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize-address-use-after-scope>
            $<$<CONFIG:DEBUG>:
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -ggdb3
            -O0
            >
            $<$<CXX_COMPILER_ID:MSVC>:
            /GL
            /LTCG
            /O3>
            >
            $<$<CONFIG:RELEASE>:
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -Ofast
            >
            >)

target_link_options(
  common INTERFACE
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address,undefined>)

# Threads needed in gcc
set(THREADS_HAVE_PTHREAD_FLAG ON)
find_package(Threads REQUIRED QUIET)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

# config
target_link_libraries(${PROJECT_NAME} PUBLIC common)

# ##############################################################################

include(FetchContent)

# libsndfile
find_package(PkgConfig REQUIRED QUIET)
pkg_check_modules(SNDFILE REQUIRED sndfile)
target_include_directories(${PROJECT_NAME} PRIVATE ${SNDFILE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SNDFILE_LIBRARIES})

# SpdLog
find_package(spdlog QUIET)
if(spdlog_FOUND)
  message(STATUS "SpdLog found , using system version")
  target_link_libraries(${PROJECT_NAME} PUBLIC spdlog)
else() # Only use this if spdlog is not installed on your system
  message(STATUS "SpdLog not found, using bundled version")
  target_include_directories(${PROJECT_NAME} SYSTEM
                             PUBLIC ${CMAKE_SOURCE_DIR}/libs/spdlog/)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC SPDLOG_USE_STD_FORMAT)

# ##############################################################################

# JSON
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.3
  SYSTEM)
FetchContent_MakeAvailable(json)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)

# CROW (api)
FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow
  GIT_TAG v1.2.0
  SYSTEM)
FetchContent_MakeAvailable(Crow)
target_link_libraries(${PROJECT_API} PUBLIC Crow::Crow)

# curl / cpr

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8
  SYSTEM)
FetchContent_MakeAvailable(cpr)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

# Ctest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571)

target_link_libraries(${PROJECT_APP} PRIVATE ${PROJECT_NAME} common cpr::cpr)
# target_link_libraries(${PROJECT_API} PRIVATE ${PROJECT_NAME} common cpr::cpr)
target_link_libraries(${PROJECT_APP_INDEXING} PRIVATE ${PROJECT_NAME} common
                                                      cpr::cpr)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
  option(PACKAGE_TESTS "Build the tests" ON)
endif()

if(${PACKAGE_TESTS})
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
endif()
