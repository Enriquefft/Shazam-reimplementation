#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_
#include "utils.hpp"
#include <complex>
#include <vector>

double PI = std::atan(1)*4;

template<typename T>
void fft(std::vector<std::complex<T>>& a) {
    size_t n = a.size();
    if (n <= 1) return;

	auto wn = std::exp(std::complex<T>(0,2*PI/n));
	auto w = std::complex<T>(1,0);

    // Divide
    std::vector<std::complex<T>> even(n / 2);
    std::vector<std::complex<T>> odd(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        even[i] = a[i*2];
        odd[i] = a[i*2 + 1];
    }

    // Conquer
    fft(even);
    fft(odd);

    // Combine
    for (size_t i = 0; i < n / 2; ++i) {
        std::complex<T> t = w * odd[i];
        a[i] = even[i] + t;
        a[i + n/2] = even[i] - t;
		w = w*wn;
    }
}

template<typename T>
std::vector<std::complex<T>> fft_real_inputs(const std::vector<T>& inputs)
{
  std::vector<std::complex<T>> inputsComplex(inputs.size());
  for (size_t i = 0; i<inputs.size();i++)
    inputsComplex[i] = std::complex<T>(inputs[i],0);
  fft(inputsComplex);
  return inputsComplex;
}

template <std::floating_point T>
constexpr auto matrix_dft(const matrix_t<T> &matrix)
    -> matrix_t<std::complex<T>> {

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

  matrix_t<std::complex<T>> result(rows, std::vector<std::complex<T>>(cols));

  // Perform 1D DFT on each row
  for (size_t k = 0; k < rows; ++k) {
    result[k] = fft_real_inputs(matrix[k]);
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
