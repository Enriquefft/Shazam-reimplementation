#ifndef INCLUDE_FFT_HPP_
#define INCLUDE_FFT_HPP_
#include <complex>
#include <vector>

/**
 * @brief Computes 1-dimensional DFT (discrete fourier transform) on given
 * input.
 *
 * @param input  an array of complex numbers, possibly a signal in time-domain.
 * @return a modifiable array to store the output in, if larger than input
 *               only elements past input length are changed and if smaller the
 *               result is truncated to output length.
 */
template <std::floating_point T>
auto dft(const std::vector<std::complex<T>> &input)
    -> std::vector<std::complex<T>>;
template <typename T>
auto dft(const std::vector<std::vector<T>> &input)
    -> std::vector<std::vector<std::complex<T>>>;

#endif // INCLUDE_FFT_HPP_
