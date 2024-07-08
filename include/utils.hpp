#ifndef INCLUDE_UTILS_HPP_
#define INCLUDE_UTILS_HPP_

#include <complex>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <sstream>
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

template <std::floating_point T>
constexpr auto format_number(const T &num) -> std::string {
  return std::to_string(num);
}
template <std::floating_point T>
auto format_number(const std::complex<T> &num) -> std::string {
  std::ostringstream oss;
  oss << '(' << std::real(num) << ", " << std::imag(num) << ')';
  return oss.str();
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
constexpr void vector_info(const std::vector<T> &vec, const std::string &name,
                           bool add_weighted_sum = false) {

  if (!info) {
    return;
  }
  T sum = sum_vector(vec);
  if (add_weighted_sum) {
    throw std::runtime_error("Weighted sum not implemented.");
  }

  std::string sum_str = format_number(sum);

  std::cout << name << " info:\tsize: " << vec.size() << ", sum: " << sum_str
            << std::endl;
}

template <typename T>
constexpr void vector_info(const matrix_t<T> &vec, const std::string &name,
                           bool add_weighted_sum = false) {
  if (!info) {
    return;
  }

  T sum = sum_vector(vec);
  if (add_weighted_sum) {
    throw std::runtime_error("Weighted sum not implemented.");
  }
  std::string sum_str = format_number(sum);

  std::cout << name << " info:\tsize: " << vec.size() << " x "
            << vec.at(0).size() << ", sum: " << sum_str << std::endl;
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
                          const size_t &begin_idx, const size_t &end_idx) {
  for (auto &row : matrix) {
    if (end_idx <= row.size()) {
      row = std::vector<T>(row.begin() + static_cast<int64_t>(begin_idx),
                           row.begin() + static_cast<int64_t>(end_idx));
    } else {
      // Handle the case where end_idx is out of bounds
      row = std::vector<T>(row.begin() + static_cast<int64_t>(begin_idx),
                           row.end());
    }
  }
}

template <std::floating_point T>
constexpr auto slice_cols(const std::vector<std::vector<T>> &matrix,
                          const size_t &begin_idx, const size_t &end_idx)
    -> std::vector<std::vector<T>> {

  auto rows = matrix.size();
  std::vector<std::vector<T>> new_matrix(rows);

  if (begin_idx > end_idx || begin_idx >= matrix.at(0).size()) {
    return new_matrix;
  }

  for (int64_t i = 0; i < static_cast<int64_t>(rows); ++i) {
    if (end_idx <= matrix[i].size()) {
      new_matrix.at(i) =
          std::vector<T>(matrix[i].begin() + static_cast<int64_t>(begin_idx),
                         matrix[i].begin() + static_cast<int64_t>(end_idx));
    } else {
      // Handle the case where end_idx is out of bounds
      new_matrix.at(i) =
          std::vector<T>(matrix[i].begin() + static_cast<int64_t>(begin_idx),
                         matrix.at(i).end());
    }
  }

  return new_matrix;
}

template <std::floating_point T>
constexpr auto multiply(const std::vector<std::vector<T>> &arr1,
                        const std::vector<std::vector<T>> &mat1)
    -> std::vector<std::vector<T>> {

  if (arr1.size() != mat1.size()) {
    throw std::runtime_error("Matrix multiplication not possible.");
  }

  if (arr1.at(0).size() != 1) {
    throw std::runtime_error(
        "Matrix multiplication not possible. (fist arg is not a column vector");
  }

  auto rows = arr1.size();
  auto cols = mat1.at(0).size();

  auto result = generate_matrix<T>({rows, cols});

  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      result.at(i).at(j) = arr1.at(i).at(0) * mat1.at(i).at(j);
    }
  }

  return result;
}

#endif // INCLUDE_UTILS_HPP_
