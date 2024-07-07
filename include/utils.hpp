#ifndef INCLUDE_UTILS_HPP_
#define INCLUDE_UTILS_HPP_

#include <complex>
#include <cstdint>
#include <format>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

static bool info = false;

template <typename T> using matrix_t = std::vector<std::vector<T>>;

template <typename T> struct IsComplexT : public std::false_type {};
template <typename T>
struct IsComplexT<std::complex<T>> : public std::true_type {};
template <typename T> constexpr auto is_complex() -> bool {
  return IsComplexT<T>::value;
}

template <typename... Args>
constexpr void print(std::string_view fmt, Args &&...args) {
  std::cout << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

template <std::floating_point T>
constexpr auto printable_complex(const std::complex<T> &num) -> std::string {
  return std::format("({}, {})", std::real(num), std::imag(num));
}

// Function to sum elements in a vector<T>
template <typename T>
constexpr auto sum_vector(const std::vector<T> &vec) -> T {
  return std::accumulate(vec.begin(), vec.end(), T{});
}

// Function to sum elements in a vector<vector<T>>
template <typename T> constexpr auto sum_vector(const matrix_t<T> &vec) -> T {
  T total_sum = T{};
  for (const auto &sub_vec : vec) {
    total_sum += sum_vector(sub_vec);
  }
  return total_sum;
}

template <typename T>
auto position_weighted_sum(const std::vector<T> &matrix) -> T {
  T sum = 0;

  for (size_t i = 0; i < matrix.size(); ++i) {
    sum += (i + 1) * matrix.at(i);
  }

  return sum;
}

template <typename T>
auto position_weighted_sum(const std::vector<std::vector<T>> &matrix) -> T {
  T sum = 0;

  for (size_t i = 0; i < matrix.size(); ++i) {
    for (size_t j = 0; j < matrix[i].size(); ++j) {
      // Weight the element by its position
      sum += (i + 1) * (j + 1) * matrix.at(i).at(j);
    }
  }

  return sum;
}

/// @brief Create an empty 2D matrix with the same dimensions.
/// @param dimensions Dimensions of the matrix.
/// @return 2D matrix of the specified dimensions.
template <typename T>
auto generate_matrix(std::pair<size_t, size_t> dimensions) -> matrix_t<T> {
  size_t rows = dimensions.first;
  size_t cols = dimensions.second;

  return matrix_t<T>(rows, std::vector<T>(cols));
}

template <typename T>
constexpr void vector_info(const std::vector<T> &vec, const std::string &name) {

  if (!info) {
    return;
  }
  std::string sum;

  if constexpr (is_complex<T>()) {
    sum = printable_complex(sum_vector(vec)); // + position_weighted_sum(vec));
  } else {
    sum = std::to_string(sum_vector(vec)); // + position_weighted_sum(vec));
  }

  print("{} info:\tsize: {}, sum: {}", name, vec.size(), sum);
}

template <typename T>
constexpr void vector_info(const matrix_t<T> &vec, const std::string &name) {
  if (!info) {
    return;
  }

  std::string sum;

  if constexpr (is_complex<T>()) {
    sum = printable_complex(sum_vector(vec)); //+ position_weighted_sum(vec));
  } else {
    sum = std::to_string(sum_vector(vec)); //+ position_weighted_sum(vec));
  }

  print("{} info:\tsize: {} x {}, sum: {}", name, vec.size(), vec.at(0).size(),
        sum);
}

template <typename T>
constexpr auto transpose(const matrix_t<T> &matrix) -> matrix_t<T> {

  if (matrix.empty()) {
    return {};
  }

  auto rows = matrix.size();
  auto cols = matrix.at(0).size();

  matrix_t<T> result(cols, std::vector<T>(rows));

  for (std::size_t i = 0; i < rows; ++i) {
    for (std::size_t j = 0; j < cols; ++j) {
      result.at(j).at(i) = matrix.at(i).at(j);
    }
  }
  return result;
}

/// @brief constexpr binary exponentiation
constexpr auto binpow(int64_t base, uint64_t exponent) -> int64_t {
  int64_t res = 1;
  while (exponent > 0) {
    if ((exponent & 1UL) != 0) {
      res = res * base;
    }
    base = base * base;
    exponent >>= 1UL;
  }
  return res;
}

template <std::floating_point T>
constexpr auto
abs(const std::vector<std::vector<std::complex<T>>> &complex_matrix)
    -> std::vector<std::vector<T>> {

  std::vector<std::vector<T>> abs_matrix(
      complex_matrix.size(), std::vector<T>(complex_matrix.at(0).size()));

  for (size_t i = 0; i < complex_matrix.size(); ++i) {
    for (size_t j = 0; j < complex_matrix.at(i).size(); ++j) {
      abs_matrix.at(i).at(j) = std::abs(complex_matrix.at(i).at(j));
    }
  }

  return abs_matrix;
}

template <std::floating_point T>
constexpr void slice_cols(std::vector<std::vector<T>> &matrix,
                          const size_t &slice_idx) {
  for (auto &row : matrix) {
    row.resize(slice_idx);
  }
}
template <std::floating_point T>
constexpr auto slice_cols(const std::vector<std::vector<T>> &matrix,
                          const size_t &slice_idx)
    -> std::vector<std::vector<T>> {

  auto rows = matrix.size();
  std::vector<std::vector<int>> new_matrix(rows, std::vector<int>(slice_idx));
  // Copy the elements to the new matrix
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < slice_idx; ++j) {
      new_matrix[i][j] = matrix[i][j];
    }
  }
}

template <std::floating_point T>
constexpr auto multiply(const std::vector<std::vector<T>> &arr1,
                        const std::vector<std::vector<T>> &mat1)
    -> std::vector<std::vector<T>> {

  std::vector<std::vector<T>> result(arr1.size(),
                                     std::vector<T>(mat1[0].size()));

  for (size_t i = 0; i < arr1.size(); ++i) {
    for (size_t j = 0; j < mat1[0].size(); ++j) {
      result[i][j] = arr1[i][0] * mat1[i][j];
    }
  }

  return result;
}

#endif // INCLUDE_UTILS_HPP_
