#include <algorithm>
#include <cmath>
#include <complex>
#include <filesystem>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

#include <AudioFile.hpp>
#include <Spectrogram.hpp>
#include <utils.hpp>

using std::complex;
using std::floating_point;
using std::pair;
using std::vector;
using std::filesystem::path;

using std::operator""i;

template <typename ANY> using matrix_t = vector<vector<ANY>>;

constexpr std::array ASSETS_PATH = {"assets/1mb.wav", "assets/5mb.wav",
                                    "assets/10mb.wav", "assets/long_wav.wav"};

template <typename T> class ResultsTest : public testing::Test {
public:
  using AudioDataType = vector<T>;
  using ComplexSpectrogramType = matrix_t<complex<T>>;
  using SpectrogramType = matrix_t<T>;

  static constexpr T COMPLEX_ERROR = static_cast<T>(1e-6);

  struct AudioProcessingData {
    size_t audiodata_shape;
    T audiodata_sum;
    pair<size_t, size_t> complex_spectrogram_shape;
    complex<T> complex_spectrogram_sum;
    pair<size_t, size_t> spectrogram_shape;
    T spectrogram_sum;
    pair<size_t, size_t> normalized_spectrogram_shape;
    T normalized_spectrogram_sum;

    AudioProcessingData() = delete;
    AudioProcessingData(size_t _audiodata_shape, T _audiodata_sum,
                        std::pair<size_t, size_t> _complex_spectrogram_shape,
                        complex<T> _complex_spectrogram_sum,
                        std::pair<size_t, size_t> _spectrogram_shape,
                        T _spectrogram_sum,
                        std::pair<size_t, size_t> _normalized_spectrogram_shape,
                        T _normalized_spectrogram_sum)
        : audiodata_shape(_audiodata_shape), audiodata_sum(_audiodata_sum),
          complex_spectrogram_shape(std::move(_complex_spectrogram_shape)),
          complex_spectrogram_sum(_complex_spectrogram_sum),
          spectrogram_shape(std::move(_spectrogram_shape)),
          spectrogram_sum(_spectrogram_sum),
          normalized_spectrogram_shape(
              std::move(_normalized_spectrogram_shape)),
          normalized_spectrogram_sum(_normalized_spectrogram_sum) {}
  };

  auto get_map(const path &song_name) -> AudioProcessingData {

    // Data generated from experiments/spectrogram/generate_matrix.py
    static const std::unordered_map<path, AudioProcessingData> SONGS_DATA = {

        {ASSETS_PATH[0],
         AudioProcessingData(
             262094, static_cast<T>(2.2597503662109375), {1025, 512},
             complex<T>{2.2729060252474556 + 0.001072374942620173i},
             {1025, 512}, static_cast<T>(54624.76675016994), {1025, 512},
             static_cast<T>(-20819216.015951935))},
        {ASSETS_PATH[1],
         AudioProcessingData(
             1322253, static_cast<T>(8.495132446289062), {1025, 2583},
             complex<T>{8.567276433158243 + 0.0021880574331410575i},
             {1025, 2583}, static_cast<T>(524434.3389410106), {1025, 2583},
             static_cast<T>(-101214680.0056647))},
        {ASSETS_PATH[2],
         AudioProcessingData(
             2635988, static_cast<T>(52.34808349609375), {1025, 5149},
             complex<T>{52.28627384948578 + 0.01103484824364466i}, {1025, 5149},
             static_cast<T>(1235600.6060180683), {1025, 5149},
             static_cast<T>(-196225434.62754357))},
        {ASSETS_PATH[3],
         AudioProcessingData(
             18113536, static_cast<T>(1163.124267578125), {1025, 35379},
             complex<T>{1163.1448364257758 + 2.879711836285999e-12i},
             {1025, 35379}, static_cast<T>(32961517.675204396), {1025, 35379},
             static_cast<T>(-733725741.707634))}};

    return SONGS_DATA.at(song_name);
  }
};

// Helper function to compare complex numbers with tolerance
template <floating_point T>
auto complex_close(const complex<T> &complex_a, const complex<T> &complex_b,
                   T tolerance = ResultsTest<T>::COMPLEX_ERROR) -> bool {

  auto real_diff = std::abs(complex_a.real() - complex_b.real());
  auto imag_diff = std::abs(complex_a.imag() - complex_b.imag());

  auto real_tol = tolerance * std::max({T{1}, std::abs(complex_a.real()),
                                        std::abs(complex_b.real())});
  auto imag_tol = tolerance * std::max({T{1}, std::abs(complex_a.imag()),
                                        std::abs(complex_b.imag())});

  return real_diff < real_tol && imag_diff < imag_tol;
}

using FloatingTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(ResultsTest, FloatingTypes);

TYPED_TEST(ResultsTest, ValidAudio) {

  for (const auto &song : ASSETS_PATH) {
    auto data = this->get_map(song);
    Audio<TypeParam> audio(song);

    // validate shape
    ASSERT_EQ(data.audiodata_shape, audio.m_audiodata.size());
    // validate sum
    ASSERT_NEAR(data.audiodata_sum, sum_vector(audio.m_audiodata), 1e-6);
  }
}

TYPED_TEST(ResultsTest, ValidComplexSpectrogram) {
  GTEST_SKIP() << "Skipping single test";

  for (const auto &song : ASSETS_PATH) {
    auto data = this->get_map(song);
    Audio<TypeParam> audio(song);

    auto complex_spectrogram = Spectrogram<TypeParam>::stft(audio);

    // validate shape
    ASSERT_EQ(data.complex_spectrogram_shape.first, complex_spectrogram.size());
    std::ranges::for_each(complex_spectrogram, [&](const auto &row) {
      ASSERT_EQ(data.complex_spectrogram_shape.second, row.size());
    });

    // validate sum
    ASSERT_TRUE(complex_close(data.complex_spectrogram_sum,
                              sum_vector(complex_spectrogram)));
  }
}

TYPED_TEST(ResultsTest, ValidSpectrogram) {
  GTEST_SKIP() << "Skipping single test";

  for (const auto &song : ASSETS_PATH) {
    auto data = this->get_map(song);
    Audio<TypeParam> audio(song);

    auto spectrogram = abs(Spectrogram<TypeParam>::stft(audio));

    // validate shape
    ASSERT_EQ(data.spectrogram_shape.first, spectrogram.size());
    std::ranges::for_each(spectrogram, [&](const auto &row) {
      ASSERT_EQ(data.spectrogram_shape.second, row.size());
    });

    // validate sum
    ASSERT_NEAR(data.spectrogram_sum, sum_vector(spectrogram), 1e-6);
  }
}

TYPED_TEST(ResultsTest, ValidNormalizedSpectrogram) {
  GTEST_SKIP() << "Skipping single test";

  for (const auto &song : ASSETS_PATH) {
    auto data = this->get_map(song);
    Audio<TypeParam> audio(song);

    auto normalized_spectrogram = Spectrogram<TypeParam>::normalize(
        abs(Spectrogram<TypeParam>::stft(audio)));

    // validate shape
    ASSERT_EQ(data.normalized_spectrogram_shape.first,
              normalized_spectrogram.size());
    std::ranges::for_each(normalized_spectrogram, [&](const auto &row) {
      ASSERT_EQ(data.normalized_spectrogram_shape.second, row.size());
    });

    // validate sum
    ASSERT_NEAR(data.normalized_spectrogram_sum,
                sum_vector(normalized_spectrogram), 1e-6);
  }
}
