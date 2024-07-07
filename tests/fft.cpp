#include "fft.hpp"
#include <cmath>
#include <complex>
#include <gtest/gtest.h>
#include <vector>

using std::complex;
using std::floating_point;
using std::vector;

template <typename T> using matrix_t = vector<vector<T>>;

template <typename T> class FftTest : public testing::Test {
public:
  using input_t = matrix_t<T>;
  using output_t = matrix_t<complex<T>>;

  auto fft(const matrix_t<T> &matrix) -> matrix_t<complex<T>> {
    return row_dft<T>(matrix);
  }
  static constexpr T COMPLEX_ERROR = static_cast<T>(1e-6F);
};

// Helper function to compare complex numbers with tolerance
template <floating_point T>
auto complex_close(const complex<T> &complex_a, const complex<T> &complex_b,
                   T tolerance = FftTest<T>::COMPLEX_ERROR) -> bool {

  auto real_diff = std::abs(complex_a.real() - complex_b.real());
  auto imag_diff = std::abs(complex_a.imag() - complex_b.imag());

  auto real_tol = tolerance * std::max({T{1}, std::abs(complex_a.real()),
                                        std::abs(complex_b.real())});
  auto imag_tol = tolerance * std::max({T{1}, std::abs(complex_a.imag()),
                                        std::abs(complex_b.imag())});

  return real_diff < real_tol && imag_diff < imag_tol;
}

using FloatingTypes = ::testing::Types<double>;
TYPED_TEST_SUITE(FftTest, FloatingTypes);

// Test case to verify the DFT of a known input
TYPED_TEST(FftTest, KnownInput) {
  typename TestFixture::input_t input = {{1, 2, 3, 4}, {4, 3, 2, 1}};

  auto result = this->fft(input);

  // Expected DFT results (manually calculated or using another trusted tool)
  typename TestFixture::output_t expected =
      {{complex<TypeParam>(5, 0), complex<TypeParam>(5, 0),
        complex<TypeParam>(5, 0), complex<TypeParam>(5, 0)},
       {complex<TypeParam>(-3, 0), complex<TypeParam>(-1, 0),
        complex<TypeParam>(1, 0), complex<TypeParam>(3, 0)}}

  ;

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < result.size(); ++i) {
    ASSERT_EQ(result[i].size(), expected[i].size());
    for (size_t j = 0; j < result[i].size(); ++j) {
      EXPECT_TRUE(complex_close(result.at(i).at(j), expected.at(i).at(j)));
    }
  }
}

// Test case to verify the DFT of an all-zero input
TYPED_TEST(FftTest, ZeroInput) {
  typename TestFixture::input_t input = {{0, 0, 0, 0}, {0, 0, 0, 0}};

  auto result = this->fft(input);

  typename TestFixture::output_t expected = {
      {complex<TypeParam>(0, 0), complex<TypeParam>(0, 0),
       complex<TypeParam>(0, 0), complex<TypeParam>(0, 0)},
      {complex<TypeParam>(0, 0), complex<TypeParam>(0, 0),
       complex<TypeParam>(0, 0), complex<TypeParam>(0, 0)}};

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < result.size(); ++i) {
    ASSERT_EQ(result[i].size(), expected[i].size());
    for (size_t j = 0; j < result[i].size(); ++j) {
      EXPECT_TRUE(complex_close(result[i][j], expected[i][j]));
    }
  }
}

// Test case to verify the DFT of a single-row input
TYPED_TEST(FftTest, SingleRowInput) {
  typename TestFixture::input_t input = {{1, -1, 1, -1}};

  auto result = this->fft(input);

  typename TestFixture::output_t expected = {
      {complex<TypeParam>(1, 0), complex<TypeParam>(-1, 0),
       complex<TypeParam>(1, 0), complex<TypeParam>(-1, 0)}};

  ASSERT_EQ(result.size(), expected.size());
  ASSERT_EQ(result[0].size(), expected[0].size());
  for (size_t j = 0; j < result[0].size(); ++j) {
    EXPECT_TRUE(complex_close(result[0][j], expected[0][j]));
  }
}

// Test case to verify the DFT of a single-column input
TYPED_TEST(FftTest, SingleColumnInput) {
  typename TestFixture::input_t input = {{1}, {-1}, {1}, {-1}};

  auto result = this->fft(input);

  typename TestFixture::output_t expected = {{complex<TypeParam>(0, 0)},
                                             {complex<TypeParam>(0, 0)},
                                             {complex<TypeParam>(4, 0)},
                                             {complex<TypeParam>(0, 0)}};

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < result.size(); ++i) {
    ASSERT_EQ(result[i].size(), expected[i].size());
    for (size_t j = 0; j < result[i].size(); ++j) {
      EXPECT_TRUE(complex_close(result[i][j], expected[i][j]));
    }
  }
}

// Explicit instantiation of the test suite

// template class FftTest_KnownInput_Test<float>;
// template class FftTest_ZeroInput_Test<float>;
// template class FftTest_SingleRowInput_Test<float>;
// template class FftTest_SingleColumnInput_Test<float>;
// template class FftTest<float>;
