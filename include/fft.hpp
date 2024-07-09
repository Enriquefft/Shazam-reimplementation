#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_
#include "utils.hpp"
#include <complex>
#include <vector>

template <typename T> constexpr void fft(std::vector<std::complex<T>> &input) {
  size_t size = input.size();
  if (size <= 1) {
    return;
  }

  auto twiddle_factor = std::exp(std::complex<T>(
      0, static_cast<T>(-2 * std::numbers::pi / static_cast<double>(size))));
  auto curr_factor = std::complex<T>(1, 0);

  // Divide
  std::vector<std::complex<T>> even(size / 2);
  std::vector<std::complex<T>> odd(size / 2);
  for (size_t i = 0; i < size / 2; ++i) {
    even[i] = input[i * 2];
    odd[i] = input[i * 2 + 1];
  }

  // Conquer
  fft(even);
  fft(odd);

  // Combine
  for (size_t i = 0; i < size / 2; ++i) {
    std::complex<T> odd_tw_factor = curr_factor * odd[i];
    input[i] = even[i] + odd_tw_factor;
    input[i + size / 2] = even[i] - odd_tw_factor;
    curr_factor = curr_factor * twiddle_factor;
  }
}

constexpr auto calculate_row_size(size_t cols) -> size_t {
  return (cols % 2 == 0) ? (cols / 2) + 1 : (cols + 1) / 2;
}

template <typename T>
constexpr auto fft_real_inputs(const std::vector<T> &inputs)
    -> std::vector<std::complex<T>> {

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
