#include <complex>
#include <numbers>
#include <vector>

using std::literals::operator""i;

template <std::floating_point T>
auto dft(const std::vector<std::complex<T>> &input)
    -> std::vector<std::complex<T>> {
  const auto M_I_2PI_DL =
      -(2i * std::numbers::pi / static_cast<T>(input.size()));

  std::vector<std::complex<T>> output;

  for (size_t k = 0; k < output.size(); ++k) {
    output[k] = 0;
    for (size_t n = 0; n < input.size(); ++n) {
      output[k] +=
          input[n] * pow(std::numbers::e,
                         M_I_2PI_DL * static_cast<T>(k) * static_cast<T>(n));
    }
  }
  return output;
}

// Overloaded DFT function to handle a vector of vectors
template <typename T>
auto dft(const std::vector<std::vector<T>> &input)
    -> std::vector<std::vector<std::complex<T>>> {

  size_t num_rows = input.size();
  size_t num_cols = input.at(0).size();

  std::vector<std::vector<std::complex<T>>> output;
  output.resize(num_rows, std::vector<std::complex<T>>(num_cols));

  for (size_t i = 0; i < num_rows; ++i) {
    std::vector<std::complex<T>> input_complex(input[i].begin(),
                                               input[i].end());
    dft(input_complex, output[i]);
  }
  return output;
}
