#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_

#include "utils.hpp"
#include <vector>
#include <complex>
#include <cmath>
#include <numbers>
#include <algorithm>

// Helper function to reverse bits
constexpr size_t reverse_bits(size_t n, size_t bits) {
  size_t reversed = 0;
  for (size_t i = 0; i < bits; ++i) {
    reversed = (reversed << 1) | (n & 1);
    n >>= 1;
  }
  return reversed;
}

// Bit-reversal permutation
template <typename T>
void bit_reversal_permutation(std::vector<std::complex<T>> &input) {
  size_t size = input.size();
  size_t bits = static_cast<size_t>(std::log2(size));
  
  for (size_t i = 0; i < size; ++i) {
    size_t reversed = reverse_bits(i, bits);
    if (i < reversed) {
      std::swap(input[i], input[reversed]);
    }
  }
}

// In-place FFT implementation with bit-reversal and loop unrolling
template <typename T>
constexpr void fft(std::vector<std::complex<T>> &input) {
  const size_t size = input.size();
  if (size <= 1) return;

  // Bit-reversal permutation
  bit_reversal_permutation(input);

  // Iterative FFT
  for (size_t len = 2; len <= size; len *= 2) {
    std::complex<T> wlen = std::exp(std::complex<T>(0, static_cast<T>(-2 * std::numbers::pi / static_cast<double>(len))));
    for (size_t i = 0; i < size; i += len) {
      std::complex<T> w(1);
      for (size_t j = 0; j < len / 2; ++j) {
        std::complex<T> u = input[i + j];
        std::complex<T> v = w * input[i + j + len / 2];
        input[i + j] = u + v;
        input[i + j + len / 2] = u - v;
        w *= wlen;
      }
    }
  }
}

constexpr auto calculate_row_size(size_t cols) -> size_t {
  return (cols % 2 == 0) ? (cols / 2) + 1 : (cols + 1) / 2;
}

template <typename T>
constexpr auto fft_real_inputs(const std::vector<T> &inputs) -> std::vector<std::complex<T>> {
  std::vector<std::complex<T>> inputs_complex(inputs.size());
  for (size_t i = 0; i < inputs.size(); i++) {
    inputs_complex[i] = std::complex<T>(inputs[i], 0);
  }
  fft(inputs_complex);
  return inputs_complex;
}

template <std::floating_point T>
constexpr auto matrix_dft(const matrix_t<T> &matrix) -> matrix_t<std::complex<T>> {
  size_t rows = matrix.size();
  matrix_t<std::complex<T>> result(rows);

  // Perform 1D DFT on each row
  for (size_t k = 0; k < rows; ++k) {
    result.at(k) = fft_real_inputs(matrix[k]);
    result.at(k).resize(calculate_row_size(matrix[k].size()));
  }

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
constexpr auto row_dft(const matrix_t<T> &matrix) -> matrix_t<std::complex<T>> {
  return transpose(matrix_dft(transpose(matrix)));
}


#endif // INCLUDE_FFT_HPP_
