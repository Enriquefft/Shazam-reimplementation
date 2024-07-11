#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_
#include "utils.hpp"
#include <complex>
#include <vector>

#include <vector>
#include <complex>
#include <cmath>
#include <numbers>


template <typename T>
constexpr void fft_recursive(std::vector<std::complex<T>> &input, const std::vector<std::complex<T>> &twiddle_factors, size_t start, size_t size, size_t stride) {
  if (size <= 1) return;

  const size_t half_size = size / 2;

  fft_recursive(input, twiddle_factors, start, half_size, stride * 2);
  fft_recursive(input, twiddle_factors, start + stride, half_size, stride * 2);

  for (size_t i = 0; i < half_size; ++i) {
    const std::complex<T> odd_tw_factor = twiddle_factors[i * stride] * input[start + (i + half_size) * stride];
    const std::complex<T> even_value = input[start + i * stride];
    input[start + i * stride] = even_value + odd_tw_factor;
    input[start + (i + half_size) * stride] = even_value - odd_tw_factor;
  }
}

constexpr auto calculate_row_size(size_t cols) -> size_t {
  return (cols % 2 == 0) ? (cols / 2) + 1 : (cols + 1) / 2;
}

// In-place FFT implementation
template <typename T>
constexpr void fft(std::vector<std::complex<T>> &input) {
  const size_t size = input.size();
  if (size <= 1) return;

  // Allocate the twiddle factors outside the recursive calls to avoid recalculating
  std::vector<std::complex<T>> twiddle_factors(size / 2);
  for (size_t i = 0; i < size / 2; ++i) {
    twiddle_factors[i] = std::exp(std::complex<T>(0, static_cast<T>(-2 * std::numbers::pi * i / static_cast<double>(size))));
  }

  fft_recursive(input, twiddle_factors, 0, size, 1);
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
constexpr auto matrix_dft(const matrix_t<T> &matrix)
    -> matrix_t<std::complex<T>> {

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
