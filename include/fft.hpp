#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_
#include "utils.hpp"
#include <complex>
#include <vector>

/**
 * @brief Computes 2-dimensional DFT (discrete fourier transform) on given
 * matrix.
 *
 * @param matrix  a matrix of complex numbers, possibly a signal in time-domain.
 * @return a complex matrixo
 */
template <std::floating_point T>
auto matrix_dft(const std::vector<std::vector<T>> &matrix)
    -> std::vector<std::vector<std::complex<T>>> {

  size_t rows = matrix.size();
  size_t cols = matrix[0].size();

  std::vector<std::vector<std::complex<T>>> result(
      rows, std::vector<std::complex<T>>(cols / 2 + 1));

  // Precompute cosine and sine values
  std::vector<std::vector<std::complex<T>>> twiddle_factors(
      cols, std::vector<std::complex<T>>(cols / 2 + 1));
  for (size_t n = 0; n <= cols / 2; ++n) {
    for (size_t t = 0; t < cols; ++t) {
      T angle = 2 * static_cast<T>(std::numbers::pi) * static_cast<T>(n) *
                static_cast<T>(t) / static_cast<T>(cols);
      twiddle_factors[t][n] =
          std::complex<T>(std::cos(angle), -std::sin(angle));
    }
  }

  for (size_t k = 0; k < rows; ++k) {
    for (size_t n = 0; n <= cols / 2; ++n) {
      std::complex<T> sum = 0;
      for (size_t t = 0; t < cols; ++t) {
        sum += std::complex<T>(matrix[k][t]) * twiddle_factors[t][n];
      }
      result[k][n] = sum;
    }
  }

  // for (size_t k = 0; k < rows; ++k) {
  //
  //   for (size_t n = 0; n <= cols / 2; ++n) {
  //     std::complex<T> sum = 0;
  //     for (size_t t = 0; t < cols; ++t) {
  //
  //       T tn = static_cast<T>(n);
  //       T tt = static_cast<T>(t);
  //       T tcols = static_cast<T>(cols);
  //
  //       T angle = 2 * static_cast<T>(std::numbers::pi) * tn * tt / tcols;
  //
  //       sum += std::complex<T>(matrix[k][t] * std::cos(angle),
  //                              -matrix[k][t] * std::sin(angle));
  //     }
  //     result[k][n] = sum;
  //   }
  // }

  return result;
}

/**
 * @brief Computes 2-dimensional DFT (discrete fourier transform) on given
 * matrix.
 *
 * @param matrix  a matrix of complex numbers, possibly a signal in time-domain.
 * @return a complex matrix
 */
template <std::floating_point T>
auto row_dft(const std::vector<std::vector<T>> &matrix)
    -> std::vector<std::vector<std::complex<T>>> {

  return transpose(matrix_dft(transpose(matrix)));
}

#endif // INCLUDE_FFT_HPP_
