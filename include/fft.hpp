#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_
#include <complex>
#include <vector>

/**
 * @brief Computes 2-dimensional DFT (discrete fourier transform) on given
 * matrix.
 *
 * @param matrix  a matrix of complex numbers, possibly a signal in time-domain.
 * @return a complex matrix
 */
template <std::floating_point T>
auto compute_dft(const std::vector<std::vector<T>> &matrix)
    -> std::vector<std::vector<std::complex<T>>>;

// Explicit instantiation

extern template auto compute_dft(const std::vector<std::vector<float>> &)
    -> std::vector<std::vector<std::complex<float>>>;
extern template auto compute_dft(const std::vector<std::vector<double>> &)
    -> std::vector<std::vector<std::complex<double>>>;

#endif // INCLUDE_FFT_HPP_
