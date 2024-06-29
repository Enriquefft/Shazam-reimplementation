{
  description =
    "Flake for A Shazam reimplementation made for the Advanced Data Structures course @utec";

  inputs = {
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
    flakelight.url = "github:nix-community/flakelight";
  };

  outputs = { flakelight, nixpkgs, ... }:
    flakelight ./. {

      inputs.nixpkgs = nixpkgs;

      devShell = pkgs: {

        stdenv = pkgs.llvmPackages_17.stdenv;

        env = {
          CC = "clang";
          CXX = "clang++";
        };

        packages = with pkgs; [

          # TODO: this is a dirty hack to make the dependencies available to pkg-config
          libsndfile

          openssl

          pkg-config
          flac
          libogg
          libvorbis
          libopus
          libmpg123

          spdlog

          nlohmann_json

          pre-commit
          commitizen

          coreutils
          clang-tools_17
          cpplint
          cmake
          cppcheck
          doxygen
          gtest
          gdb

          include-what-you-use

        ];

      };
    };

}
