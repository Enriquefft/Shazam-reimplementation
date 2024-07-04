#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_
#include "utils.hpp"
#include <complex>
#include <vector>

#include <vector>
#include <complex>
#include <cmath>
#include <numbers> // Requires C++20 for std::numbers::pi

template <std::floating_point T>
auto dft(const std::vector<T> &input)
    -> std::vector<std::complex<T>> {

    size_t N = input.size();
    std::vector<std::complex<T>> result(N);

    for (size_t n = 0; n < N; ++n) {
        std::complex<T> sum = 0;
        for (size_t k = 0; k < N; ++k) {
            T angle = 2 * static_cast<T>(std::numbers::pi) * static_cast<T>(n) * static_cast<T>(k) / static_cast<T>(N);
            sum += std::complex<T>(std::cos(angle), -std::sin(angle)) * input[k];
        }
        result[n] = sum;
    }

    return result;
}

template <typename T>
void fft_recursive(std::vector<std::complex<T>>& input, bool inverse = false) {
    size_t N = input.size();
    if (N <= 1) return;

    std::vector<std::complex<T>> even(N / 2), odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = input[2*i];
        odd[i] = input[2*i + 1];
    }

    fft_recursive(even, inverse);
    fft_recursive(odd, inverse);

    T angle = (inverse ? 2 : -2) * std::numbers::pi_v<T> / N;
    std::complex<T> w(1), wn(std::cos(angle), std::sin(angle));
    for (size_t k = 0; k < N / 2; ++k) {
        input[k] = even[k] + w * odd[k];
        input[k + N / 2] = even[k] - w * odd[k];
        if (inverse) {
            input[k] /= 2;
            input[k + N / 2] /= 2;
        }
        w *= wn;
    }
}

template <std::floating_point T>
auto fft(const std::vector<T>& input) -> std::vector<std::complex<T>> {
    size_t N = input.size();
    std::vector<std::complex<T>> data(N);
    for (size_t i = 0; i < N; ++i) {
        data[i] = std::complex<T>(input[i], 0);
    }

    fft_recursive(data);

    return data;
}

template <std::floating_point T>
auto matrix_dft(const std::vector<std::vector<T>> &matrix)
    -> std::vector<std::vector<std::complex<T>>> {

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

    std::vector<std::vector<std::complex<T>>> result(rows, std::vector<std::complex<T>>(cols));

    // Perform 1D DFT on each row
    for (size_t k = 0; k < rows; ++k) {
        result[k] = fft(matrix[k]);
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
auto row_dft(const std::vector<std::vector<T>> &matrix)
    -> std::vector<std::vector<std::complex<T>>> {

  return transpose(matrix_dft(transpose(matrix)));
}

#endif // INCLUDE_FFT_HPP_
