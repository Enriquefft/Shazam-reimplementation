# Shazam Reimplementation

A Shazam reimplementation made for the Advanced Data Structures course @utec

## Table of contents

<!--toc:start-->

- [Shazam Reimplementation](#shazam-reimplementation)
  - [Table of contents](#table-of-contents)
  - [Installation](#installation)
    - [Dependencies](#dependencies)
    - [Running](#running)
  - [Contributing](#contributing)
    - [Pre-commit hooks](#pre-commit-hooks)
  - [Objectives](#objectives)
    - [Main](#main)
    - [Secondary](#secondary)
  - [Results](#results)
  - [Conclusions](#conclusions)
  - [Authors](#authors)
  - [Bibliography](#bibliography)
  <!--toc:end-->

## Installation

**_Windows is not supported by default_**

### Dependencies

- [spdlog](https://github.com/gabime/spdlog)
- [CrowCpp](https://github.com/CrowCpp/Crow)
- [libsndfile](https://github.com/libsndfile/libsndfile)
<!--TODO: revisit after db connection-->
- [libpqxx](https://github.com/jtv/libpqxx) (probably, not yet implemented)

### Running

```
cmake -S . -B build
cmake --build build
```

## Contributing

### Pre-commit hooks

These tools are used as part of the pre-commit tests, they can be disabled by editing `.pre-commit-config.yaml` (or `CMakeLists.txt` in the case of [IWYU](https://github.com/include-what-you-use/include-what-you-use?tab=readme-ov-file#using-with-cmake))

- clang-format
- clang-tidy
- cppcheck
- cpplint
- include-what-you-use
- commitizen
  commit message standardizer

## Objectives

### Main

- 1

### Secondary

- 1.1
- 1.2

## Results

## Conclusions

- C1
- C2
- C3

## Authors

<table>
    <tr >
        <th style="width:24%;">P1</th>
        <th style="width:24%;">P1</th>
        <th style="width:24%;">Nombre larguisisisimoooo</th>
        <th style="width:24%;">P1</th>
    </tr>
    <tr >
        <td><a href="https://github.com/AaronCS25"><img src="https://avatars.githubusercontent.com/u/102536323?s=400&v=4"></a></td>
        <td><a href="https://github.com/Enriquefft"><img src="https://avatars.githubusercontent.com/u/60308719?v=4"></a></td>
        <td><a href="https://github.com/AaronCS25"><img src="https://avatars.githubusercontent.com/u/102536323?s=400&v=4"></a></td>
        <td><a href="https://github.com/Enriquefft"><img src="https://avatars.githubusercontent.com/u/60308719?v=4"></a></td>
    </tr>
</table>

## Bibliography

- b1
- b2
- b3
