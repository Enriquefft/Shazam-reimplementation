#include "fft.hpp"
#include <complex>
#include <concepts>
#include <numbers>
#include <vector>

using std::complex;
using std::vector;

template <typename T> using matrix_t = vector<vector<T>>;

template <std::floating_point T>
auto compute_dft(const matrix_t<T> &matrix) -> vector<vector<complex<T>>> {

  size_t rows = matrix.size();
  size_t cols = matrix[0].size();

  matrix_t<complex<T>> result(rows, vector<complex<T>>(cols / 2 + 1));

  for (size_t k = 0; k < rows; ++k) {
    for (size_t n = 0; n <= cols / 2; ++n) {
      complex<double> sum = 0;
      for (size_t t = 0; t < cols; ++t) {

        T angle = 2 * std::numbers::pi * n * t / cols;
        sum += complex<double>(matrix[k][t] * cos(angle),
                               -matrix[k][t] * sin(angle));
      }
      result[k][n] = sum;
    }
  }

  return result;
}
// Explicit instantiation

template auto compute_dft(const matrix_t<float> &) -> matrix_t<complex<float>>;
template auto compute_dft(const matrix_t<double> &)
    -> matrix_t<complex<double>>;
