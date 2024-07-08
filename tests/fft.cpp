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
  GTEST_SKIP();
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
  GTEST_SKIP();
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
  GTEST_SKIP();
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
  GTEST_SKIP();
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

TYPED_TEST(FftTest, TallMatrixInput) {
  GTEST_SKIP();
  typename TestFixture::input_t input = {{1, 2}, {3, 4}, {4, 3}, {2, 1}};

  auto result = this->fft(input);

  typename TestFixture::output_t expected = {
      {complex<TypeParam>(10, 0), complex<TypeParam>(10, 0)},
      {complex<TypeParam>(-3, -1), complex<TypeParam>(-1, -3)},
      {complex<TypeParam>(0, 0), complex<TypeParam>(0, 0)}};

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < result.size(); ++i) {
    ASSERT_EQ(result[i].size(), expected[i].size());
    for (size_t j = 0; j < result[i].size(); ++j) {
      EXPECT_TRUE(complex_close(result[i][j], expected[i][j]));
    }
  }
}

TYPED_TEST(FftTest, SmallInput) {

  typename TestFixture::input_t input =

      {
          {
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
              0,
          },
          {
              -2.78035e-07,
              -1.181e-07,
              -3.68318e-07,
              -3.90817e-07,
              -4.34161e-08,
              -1.1005e-07,
              -1.12781e-07,
              -3.62136e-07,
              -6.31115e-07,
              -2.62797e-07,
              -4.64495e-07,
              -1.21479e-08,
              2.55896e-07,
              4.59894e-07,
              7.34623e-08,
              2.32679e-07,
          },
          {
              -1.15412e-06,
              -4.41348e-07,
              -1.3965e-06,
              -1.49742e-06,
              -2.71997e-07,
              -5.44856e-07,
              -4.38185e-07,
              -1.48189e-06,
              -2.51353e-06,
              -1.06959e-06,
              -1.86027e-06,
              -1.41461e-07,
              9.6579e-07,
              1.86459e-06,
              2.42957e-07,
              8.71195e-07,
          },
          {
              -2.6705e-06,
              -9.43216e-07,
              -2.86588e-06,
              -3.1667e-06,
              -8.48765e-07,
              -1.43811e-06,
              -9.56801e-07,
              -3.3226e-06,
              -5.5946e-06,
              -2.4085e-06,
              -4.11315e-06,
              -4.61257e-07,
              2.06434e-06,
              4.25806e-06,
              4.95544e-07,
              1.75511e-06,
          },
          {
              -4.88785e-06,
              -1.51811e-06,
              -4.58768e-06,
              -5.27542e-06,
              -1.80678e-06,
              -2.98676e-06,
              -1.77113e-06,
              -5.83666e-06,
              -9.94361e-06,
              -4.22425e-06,
              -7.40078e-06,
              -9.63768e-07,
              3.5043e-06,
              7.67104e-06,
              8.4646e-07,
              2.68084e-06,
          },
          {
              -7.92652e-06,
              -2.02161e-06,
              -6.52307e-06,
              -7.68752e-06,
              -3.08004e-06,
              -5.24541e-06,
              -2.9273e-06,
              -9.12152e-06,
              -1.59016e-05,
              -6.73152e-06,
              -1.2162e-05,
              -1.65323e-06,
              5.28854e-06,
              1.21872e-05,
              1.40165e-06,
              3.61015e-06,
          },
          {
              -1.15668e-05,
              -2.66268e-06,
              -8.58321e-06,
              -1.04127e-05,
              -4.82854e-06,
              -8.30633e-06,
              -4.58013e-06,
              -1.30365e-05,
              -2.36329e-05,
              -1.00401e-05,
              -1.84266e-05,
              -2.29006e-06,
              7.29715e-06,
              1.77461e-05,
              2.33147e-06,
              4.51285e-06,
          },
          {
              -1.55674e-05,
              -3.94116e-06,
              -1.05097e-05,
              -1.36479e-05,
              -7.20608e-06,
              -1.28554e-05,
              -7.07929e-06,
              -1.74059e-05,
              -3.30367e-05,
              -1.38064e-05,
              -2.59363e-05,
              -2.36681e-06,
              9.35804e-06,
              2.4052e-05,
              3.61713e-06,
              5.2267e-06,
          },
          {
              -2.01165e-05,
              -5.902e-06,
              -1.20846e-05,
              -1.72644e-05,
              -1.02215e-05,
              -1.87364e-05,
              -1.07092e-05,
              -2.28904e-05,
              -4.40373e-05,
              -1.8051e-05,
              -3.43861e-05,
              -1.98267e-06,
              1.15878e-05,
              3.0913e-05,
              5.15217e-06,
              5.60299e-06,
          },
          {
              -2.5314e-05,
              -8.27887e-06,
              -1.39844e-05,
              -2.15064e-05,
              -1.40543e-05,
              -2.55061e-05,
              -1.54283e-05,
              -2.98086e-05,
              -5.6648e-05,
              -2.25719e-05,
              -4.36766e-05,
              -1.68838e-06,
              1.40135e-05,
              3.83203e-05,
              6.94564e-06,
              5.30383e-06,
          },
          {
              -3.1582e-05,
              -1.09394e-05,
              -1.68116e-05,
              -2.77511e-05,
              -1.85367e-05,
              -3.42558e-05,
              -2.07073e-05,
              -3.73824e-05,
              -7.12429e-05,
              -2.68383e-05,
              -5.41796e-05,
              -1.27938e-06,
              1.59491e-05,
              4.63596e-05,
              9.06348e-06,
              3.85971e-06,
          },
          {
              -3.91442e-05,
              -1.39844e-05,
              -1.89677e-05,
              -3.62829e-05,
              -2.39074e-05,
              -4.49014e-05,
              -2.65165e-05,
              -4.5458e-05,
              -8.78722e-05,
              -3.14388e-05,
              -6.48519e-05,
              -8.95768e-07,
              1.70892e-05,
              5.49549e-05,
              1.13319e-05,
              1.40888e-06,
          },
          {
              -4.71532e-05,
              -1.81948e-05,
              -1.91573e-05,
              -4.55801e-05,
              -3.078e-05,
              -5.6292e-05,
              -3.26429e-05,
              -5.45118e-05,
              -0.000106033,
              -3.63067e-05,
              -7.39279e-05,
              -1.41791e-06,
              1.88985e-05,
              6.37541e-05,
              1.31338e-05,
              -1.6663e-06,
          },
          {
              -5.63952e-05,
              -2.37095e-05,
              -1.80008e-05,
              -5.48769e-05,
              -3.77749e-05,
              -6.83714e-05,
              -3.82122e-05,
              -6.46789e-05,
              -0.000125204,
              -4.06414e-05,
              -8.25097e-05,
              -2.22277e-06,
              2.23856e-05,
              7.28777e-05,
              1.40775e-05,
              -5.81807e-06,
          },
          {
              -6.76293e-05,
              -3.04128e-05,
              -1.40865e-05,
              -6.35583e-05,
              -4.34005e-05,
              -8.02368e-05,
              -4.50205e-05,
              -7.5236e-05,
              -0.000144049,
              -4.59502e-05,
              -9.21258e-05,
              -2.8173e-06,
              2.61023e-05,
              8.30118e-05,
              1.42555e-05,
              -1.16777e-05,
          },
          {
              -7.80705e-05,
              -4.04098e-05,
              -5.80516e-06,
              -7.01471e-05,
              -4.83656e-05,
              -9.06187e-05,
              -5.32005e-05,
              -8.60425e-05,
              -0.000163143,
              -5.38958e-05,
              -0.000102569,
              -3.54131e-06,
              2.94786e-05,
              9.3869e-05,
              1.35831e-05,
              -1.92912e-05,
          },
          {
              -8.6617e-05,
              -5.39241e-05,
              5.0962e-06,
              -7.50632e-05,
              -5.29858e-05,
              -9.76373e-05,
              -5.85787e-05,
              -9.89068e-05,
              -0.000185119,
              -6.52203e-05,
              -0.000114232,
              -2.79647e-06,
              3.4036e-05,
              0.000105125,
              1.14619e-05,
              -2.82591e-05,
          },
          {
              -9.54538e-05,
              -6.81012e-05,
              1.81936e-05,
              -7.93995e-05,
              -5.78414e-05,
              -9.99399e-05,
              -6.24728e-05,
              -0.000111882,
              -0.000206817,
              -8.09572e-05,
              -0.000126067,
              -1.70305e-06,
              3.89624e-05,
              0.000118611,
              6.95758e-06,
              -3.83602e-05,
          },
          {
              -0.000103914,
              -8.44959e-05,
              3.36913e-05,
              -8.21675e-05,
              -6.29819e-05,
              -0.000102028,
              -6.79879e-05,
              -0.000121121,
              -0.000224639,
              -9.83729e-05,
              -0.000135906,
              -3.11999e-06,
              4.28882e-05,
              0.000134509,
              -1.86268e-07,
              -5.04554e-05,
          },
          {
              -0.000110589,
              -0.00010579,
              4.87444e-05,
              -8.36879e-05,
              -6.64367e-05,
              -0.000107113,
              -7.32593e-05,
              -0.000131421,
              -0.000243567,
              -0.000113988,
              -0.000147012,
              -2.77576e-06,
              4.628e-05,
              0.000150384,
              -9.15742e-06,
              -6.56065e-05,
          },
          {
              -0.00011575,
              -0.000129288,
              6.24306e-05,
              -8.64888e-05,
              -6.9329e-05,
              -0.000112444,
              -8.07401e-05,
              -0.000144637,
              -0.00026579,
              -0.000130236,
              -0.000164872,
              2.64439e-06,
              4.65643e-05,
              0.000167459,
              -2.00629e-05,
              -8.37007e-05,
          },
          {
              -0.000119529,
              -0.000153816,
              7.45948e-05,
              -9.26256e-05,
              -7.36125e-05,
              -0.000121019,
              -9.10729e-05,
              -0.00015201,
              -0.000287922,
              -0.000149728,
              -0.000189244,
              1.00453e-05,
              4.1005e-05,
              0.00018766,
              -3.37166e-05,
              -0.000103526,
          },
          {
              -0.000123459,
              -0.000180737,
              8.27001e-05,
              -0.000102697,
              -7.73792e-05,
              -0.000136709,
              -9.96366e-05,
              -0.000154619,
              -0.000310526,
              -0.000170686,
              -0.00021534,
              1.93013e-05,
              3.2621e-05,
              0.000207828,
              -5.05313e-05,
              -0.000122868,
          },
          {
              -0.000128813,
              -0.000207341,
              8.78014e-05,
              -0.0001138,
              -8.30882e-05,
              -0.000154584,
              -0.000111405,
              -0.000158233,
              -0.000333341,
              -0.000190844,
              -0.000240295,
              2.93812e-05,
              2.43259e-05,
              0.000228018,
              -7.32058e-05,
              -0.000141204,
          },
          {
              -0.000134873,
              -0.000230761,
              9.35709e-05,
              -0.000122913,
              -9.53918e-05,
              -0.000171954,
              -0.000128748,
              -0.0001578,
              -0.000356281,
              -0.000209324,
              -0.000266435,
              3.56736e-05,
              1.63884e-05,
              0.00024955,
              -0.000103379,
              -0.000162518,
          },
          {
              -0.00014194,
              -0.000251864,
              9.80245e-05,
              -0.00013296,
              -0.000111136,
              -0.000192636,
              -0.00014432,
              -0.000155456,
              -0.000384105,
              -0.000226449,
              -0.000297262,
              3.66862e-05,
              8.8011e-06,
              0.000267715,
              -0.00013525,
              -0.000189942,
          },
          {
              -0.000152011,
              -0.0002726,
              9.32463e-05,
              -0.000146766,
              -0.00012632,
              -0.00021942,
              -0.000160947,
              -0.000157256,
              -0.00041344,
              -0.000243557,
              -0.000332287,
              3.13249e-05,
              -9.71315e-08,
              0.000283236,
              -0.000162987,
              -0.000220003,
          },
          {
              -0.000169578,
              -0.00029396,
              7.77713e-05,
              -0.000163032,
              -0.000141978,
              -0.000250282,
              -0.000181414,
              -0.000158161,
              -0.000439028,
              -0.000263427,
              -0.000367751,
              2.50335e-05,
              -1.00029e-05,
              0.000302182,
              -0.000188013,
              -0.000246773,
          },
          {
              -0.00019774,
              -0.000316744,
              6.61197e-05,
              -0.000180675,
              -0.000160568,
              -0.000279291,
              -0.000200894,
              -0.000158879,
              -0.00046464,
              -0.000286162,
              -0.000397563,
              3.17082e-05,
              -2.31475e-05,
              0.000321418,
              -0.000210862,
              -0.000268647,
          },
          {
              -0.000230109,
              -0.000344832,
              7.06821e-05,
              -0.000198453,
              -0.000181599,
              -0.000305926,
              -0.00022292,
              -0.000161421,
              -0.000490183,
              -0.000305685,
              -0.00041636,
              5.02628e-05,
              -4.68797e-05,
              0.000338489,
              -0.000228297,
              -0.000290219,
          },
          {
              -0.000257167,
              -0.000380901,
              8.47525e-05,
              -0.000218443,
              -0.000200018,
              -0.000331123,
              -0.000249732,
              -0.000160002,
              -0.00051498,
              -0.000320262,
              -0.000429193,
              6.71684e-05,
              -7.69948e-05,
              0.000358663,
              -0.000241328,
              -0.000317159,
          },
          {
              -0.000274307,
              -0.000422643,
              0.000102296,
              -0.000246904,
              -0.000217568,
              -0.00035065,
              -0.000278863,
              -0.000157999,
              -0.000544473,
              -0.000333669,
              -0.000447631,
              7.92413e-05,
              -0.000104298,
              0.000382056,
              -0.00025291,
              -0.000349131,
          },
      };

  auto result_first_row = (this->fft(input)).at(0);

  vector<complex<TypeParam>> expected_first_row = {
      complex<TypeParam>(-0.00285131, 0), complex<TypeParam>(-0.00366128, 0),
      complex<TypeParam>(0.00094598, 0),  complex<TypeParam>(-0.00250512, 0),
      complex<TypeParam>(-0.00202336, 0), complex<TypeParam>(-0.00348316, 0),
      complex<TypeParam>(-0.00247329, 0), complex<TypeParam>(-0.00280638, 0),
      complex<TypeParam>(-0.00655433, 0), complex<TypeParam>(-0.0035028, 0),
      complex<TypeParam>(-0.00490604, 0), complex<TypeParam>(0.00038434, 0),
      complex<TypeParam>(0.00025469, 0),  complex<TypeParam>(0.0043552, 0),
      complex<TypeParam>(-0.00159492, 0), complex<TypeParam>(-0.00268176, 0)};

  ASSERT_EQ(result_first_row.size(), expected_first_row.size());
  for (size_t i = 0; i < result_first_row.size(); ++i) {
    EXPECT_TRUE(complex_close(result_first_row[i], expected_first_row[i]));
  }
}

// Explicit instantiation of the test suite

// template class FftTest_KnownInput_Test<float>;
// template class FftTest_ZeroInput_Test<float>;
// template class FftTest_SingleRowInput_Test<float>;
// template class FftTest_SingleColumnInput_Test<float>;
// template class FftTest<float>;
