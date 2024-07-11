#include "Spectrogram.hpp"
#include <algorithm>
#include <concepts>
#include <deque>
#include <limits>
#include <sndfile.h>

using std::floating_point;
using std::numeric_limits;
using std::vector;

template <typename T> using matrix_t = vector<vector<T>>;

template <floating_point T> auto Spectrogram<T>::get_x() -> size_t {
  // get dimensions of spectrogram
  return m_spectrogram.size();
}

template <floating_point T> auto Spectrogram<T>::get_y() -> size_t {
  size_t sp_x = m_spectrogram.size();
  return sp_x > 0 ? m_spectrogram[0].size() : 0;
}

template <floating_point T>
auto Spectrogram<T>::get_feature_count() -> size_t
{
  return m_features.size();
}

template <floating_point T>
inline auto Spectrogram<T>::is_max_in_neighborhood(
    size_t max_x, size_t max_y, size_t current_x, size_t current_y,
    int neighborhood_size, intensity_t threshold, const spdata_t &sp_data)
    -> bool {
  // clip the overhangs of the neighbourhood
  uint xlo = static_cast<uint>(
      std::max(0, static_cast<int>(current_x) - neighborhood_size));
  uint xhi = static_cast<uint>(
      std::min(max_x, static_cast<size_t>(current_x) +
                          static_cast<size_t>(neighborhood_size)));

  uint ylo = static_cast<uint>(
      std::max(0, static_cast<int>(current_y) - neighborhood_size));
  uint yhi = static_cast<uint>(
      std::min(max_y, static_cast<size_t>(current_y) +
                          static_cast<size_t>(neighborhood_size)));

  for (uint i = xlo; i < xhi; i++) {
    for (uint j = ylo; j < yhi; j++) {
      //  is not the same point
      if (current_x == i && current_y == j) {
        continue;
      }
      if (std::max(T{}, sp_data[i][j] - threshold) >=
          std::max(T{}, sp_data[current_x][current_y] - threshold)) {
        return false;
      }
    }
  }
  return true;
}

template <floating_point T>
inline auto Spectrogram<T>::peak_filter_minlist(const intensity_t &maxd,
                                                const intensity_t &spd)
    -> bool {
  return (maxd - spd) <= numeric_limits<T>::epsilon() &&
         spd >= numeric_limits<T>::epsilon(); // not 0
}

template <floating_point T>
inline auto Spectrogram<T>::peak_filter_minlist_gtn(
    const size_t &current_x, const size_t &current_y,
    const spdata_t &main_filter, const spdata_t &sp_data, const size_t &max_x,
    const size_t &max_y, const int &neighborhood_size,
    const intensity_t &thresh) -> bool {
  bool is_candidate =
      ((main_filter[current_x][current_y] - sp_data[current_x][current_y]) <=
       numeric_limits<T>::epsilon()) &&
      // NOLINTNEXTLINE
      sp_data[current_x][current_y] != 0;

  if (is_candidate) {
    return is_max_in_neighborhood(max_x, max_y, current_x, current_y,
                                  neighborhood_size, thresh, sp_data);
  }
  return false;
}

template <floating_point T>
auto Spectrogram<T>::max_in_neighborhood(size_t max_x, size_t max_y,
                                         uint current_x, uint current_y,
                                         int neighborhood_size,
                                         const spdata_t &sp_data)
    -> intensity_t {
  // clip the overhangs of the neighbourhood
  uint xlo = static_cast<uint>(
      std::max(0, static_cast<int>(current_x) - neighborhood_size));
  uint xhi = static_cast<uint>(
      std::min(max_x, static_cast<size_t>(current_x) +
                          static_cast<size_t>(neighborhood_size)));

  uint ylo = static_cast<uint>(
      std::max(0, static_cast<int>(current_y) - neighborhood_size));
  uint yhi = static_cast<uint>(
      std::min(max_y, static_cast<size_t>(current_y) +
                          static_cast<size_t>(neighborhood_size)));

  intensity_t max = sp_data[xlo][ylo];
  for (uint i = xlo; i < xhi; i++) {
    for (uint j = ylo; j < yhi; j++) {
      if (sp_data[i][j] > max) {
        max = sp_data[i][j];
      }
    }
  }
  return max;
}

template <floating_point T>
void Spectrogram<T>::maxfilter_x(spdata_t &maxfiltered_spectrogram,
                                 const spdata_t &spectrogram, size_t sp_x,
                                 size_t sp_y, int neigh) {
  std::deque<size_t> wedge;
  size_t w_size = 2 * static_cast<size_t>(neigh) + 1UL;

  for (size_t frame = 0; frame < sp_x; ++frame) {
    size_t y_start = 0;
    size_t y_read = 0;
    size_t y_write = 0;

    for (; y_read < static_cast<size_t>(neigh) && y_read < sp_y; ++y_read) {
      while (!wedge.empty() &&
             spectrogram[frame][wedge.back()] <= spectrogram[frame][y_read]) {
        wedge.pop_back();
      }
      wedge.push_back(y_read);
    }

    for (; y_read < sp_y; ++y_read, ++y_write) {
      while (!wedge.empty() &&
             spectrogram[frame][wedge.back()] <= spectrogram[frame][y_read]) {
        wedge.pop_back();
      }
      wedge.push_back(y_read);
      maxfiltered_spectrogram[frame][y_write] =
          spectrogram[frame][wedge.front()];
      if (y_read + 1 >= w_size) {
        if (wedge.front() == y_start) {
          wedge.pop_front();
        }
        ++y_start;
      }
    }

    for (; y_write < sp_y; ++y_write) {
      maxfiltered_spectrogram[frame][y_write] =
          spectrogram[frame][wedge.front()];
      if (wedge.front() == y_start) {
        wedge.pop_front();
      }
      ++y_start;
    }
    wedge.clear();
  }
}

template <floating_point T>
void Spectrogram<T>::maxfilter_y(spdata_t &maxfiltered_spectrogram, size_t sp_x,
                                 size_t sp_y, int neigh) {
  std::deque<size_t> wedge;
  size_t w_size = 2 * static_cast<size_t>(neigh) + 1UL;

  for (size_t freq = 0; freq < sp_y; ++freq) {
    size_t x_read = 0;
    size_t x_write = 0;

    // Initialize wedge for the first window
    for (; x_read < w_size && x_read < sp_x; ++x_read) {
      while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][freq] <=
                                   maxfiltered_spectrogram[x_read][freq]) {
        wedge.pop_back();
      }
      wedge.push_back(x_read);
    }

    // Process each column
    for (; x_read < sp_x; ++x_read, ++x_write) {
      // Remove elements outside the window
      while (!wedge.empty() && wedge.front() <= x_read - w_size) {
        wedge.pop_front();
      }

      // Add the current element to the wedge
      while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][freq] <=
                                   maxfiltered_spectrogram[x_read][freq]) {
        wedge.pop_back();
      }
      wedge.push_back(x_read);

      // Update the maxfiltered_spectrogram
      maxfiltered_spectrogram[x_write][freq] =
          maxfiltered_spectrogram[wedge.front()][freq];
    }

    // Finalize the remaining columns
    for (; x_write < sp_x; ++x_write) {
      // Remove elements outside the window
      while (!wedge.empty() && wedge.front() <= x_write - w_size) {
        wedge.pop_front();
      }

      // Update the maxfiltered_spectrogram
      maxfiltered_spectrogram[x_write][freq] =
          maxfiltered_spectrogram[wedge.front()][freq];
    }

    // Clear wedge for the next row
    wedge.clear();
  }
}

template <floating_point T>
auto Spectrogram<T>::get_local_maximums() -> std::vector<DataPoint> {

  constexpr auto MAX_FILTER = 60;
  constexpr auto GTN_WINDOW_SIZE = 3;
  constexpr T MAXIMA_THRESHOLD = static_cast<T>(0.5);

  // this is an API function that calls -some- algorithm that returns the local
  // maxima. the idea is to allow hyperparameter tuning that was not defined in
  // the API

  // return maxima_gtn_algorithm(30,0.5f);
  m_features = maxima_minlistgcn_algorithm(MAX_FILTER, GTN_WINDOW_SIZE,
                                           MAXIMA_THRESHOLD);
  return m_features;
  // return maxima_minlist_algorithm_optimized(MAX_FILTER);
}




template <floating_point T>
auto Spectrogram<T>::maxima_minlist_algorithm(int neigh) -> CritSet_t {
  size_t sp_x = 0;
  size_t sp_y = 0;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;
  spdata_t data(sp_x, spcol_t(sp_y));

  CritSet_t dat;

  for (uint32_t i = 0; i < sp_x; i++) {
    for (uint32_t j = 0; j < sp_y; j++) {
      intensity_t maxfiltered =
          max_in_neighborhood(sp_x, sp_y, i, j, neigh, m_spectrogram);
      data[i][j] = maxfiltered;

      if (peak_filter_minlist(maxfiltered, m_spectrogram[i][j])) {
        dat.push_back(
            DataPoint{i, static_cast<time_t>(j), m_spectrogram[i][j]});
      }
    }
  }

  return dat;
}

// Original function modified to use the above helper functions
template <floating_point T>
auto Spectrogram<T>::maxima_minlist_algorithm_optimized(int neigh)
    -> CritSet_t {

  size_t sp_x = m_spectrogram.size();
  size_t sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;
  spdata_t maxf_sp(sp_x, spcol_t(sp_y));

  // Apply max filters (the way they are implemented forces this order :/)
  maxfilter_x(maxf_sp, m_spectrogram, sp_x, sp_y, neigh);
  maxfilter_y(maxf_sp, sp_x, sp_y, neigh);

  CritSet_t dat;

  for (uint32_t i = 0; i < sp_x; i++) {
    for (uint32_t j = 0; j < sp_y; j++) {
      if (peak_filter_minlist(maxf_sp[i][j], m_spectrogram[i][j])) {
        dat.push_back(DataPoint{i, j, m_spectrogram[i][j]});
      }
    }
  }

  return dat; // Return the result as needed
}

template <floating_point T>
auto Spectrogram<T>::maxima_minlistgcn_algorithm(int maxfilter_s, int gtn_s,
                                                 intensity_t thresh)
    -> CritSet_t {

  size_t sp_x = m_spectrogram.size();
  size_t sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;
  spdata_t maxf_sp(sp_x, spcol_t(sp_y));

  // Apply max filters (the way they are implemented forces this order :/)
  maxfilter_x(maxf_sp, m_spectrogram, sp_x, sp_y, maxfilter_s);
  maxfilter_y(maxf_sp, sp_x, sp_y, maxfilter_s);

  // find mean loudness of the smaxfiltered spectrogram
  double avg_loudness_d = 0;
  size_t num_elements = sp_x * sp_y;
  for (uint i = 0; i < sp_x; i++) {
    for (uint j = 0; j < sp_y; j++) {
      avg_loudness_d = (static_cast<double>(num_elements) /
                        static_cast<double>(num_elements + 1)) *
                           avg_loudness_d +
                       (static_cast<double>(maxf_sp[i][j]) /
                        static_cast<double>(num_elements + 1));
    }
  }
  auto avg_loudness = static_cast<intensity_t>(avg_loudness_d);

  CritSet_t dat;

  for (uint32_t i = 0; i < sp_x; i++) {
    for (uint32_t j = 0; j < sp_y; j++) {
      if (peak_filter_minlist_gtn(i, j, maxf_sp, m_spectrogram, sp_x, sp_y,
                                  gtn_s, avg_loudness * thresh)) {
        dat.push_back(DataPoint{i, j, m_spectrogram[i][j]});
      }
    }
  }

  return dat; // Return the result as needed
}

template <floating_point T>
auto Spectrogram<T>::maxima_gtn_algorithm(int neighbourhood, T thrsh)
    -> CritSet_t {
  size_t sp_x = 0;
  size_t sp_y = 0;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;

  CritSet_t dat;

  // find mean loudness of the spectrogram
  T avg_loudness_d = 0;
  size_t num_elements = sp_x * sp_y;
  for (size_t i = 0; i < sp_x; i++) {
    for (size_t j = 0; j < sp_y; j++) {
      avg_loudness_d =
          (static_cast<T>(num_elements) / static_cast<T>(num_elements + 1)) *
              avg_loudness_d +
          (m_spectrogram[i][j] / static_cast<T>(num_elements + 1));
    }
  }
  T avg_loudness = avg_loudness_d;
  for (uint i = 0; i < sp_x; i++) {
    for (uint j = 0; j < sp_y; j++) {
      // detect candidate local maxima
      if (is_max_in_neighborhood(sp_x, sp_y, i, j, neighbourhood,
                                 avg_loudness * thrsh, m_spectrogram)) {
        dat.push_back(DataPoint{static_cast<hertz_t>(i), static_cast<time_t>(j),
                                m_spectrogram[i][j]});
      }
    }
  }
  return dat;
}

///////////////////////////////////// HASHING
///////////////////////////////////////////

constexpr auto assemble_hash(uint32_t hash_1, uint32_t hash_2, uint32_t delta_t)
    -> uint32_t {
  // this has last 10 bits on
  constexpr uint16_t MASK = 0x3FF;

  hash_1 = (hash_1 & MASK) << 20;
  hash_2 = (hash_2 & MASK) << 10;
  delta_t = delta_t & MASK;
  return hash_1 | hash_2 | delta_t;
}

template <std::floating_point T>
auto Spectrogram<T>::select_pivots_naive(
    const std::vector<typename Spectrogram<T>::DataPoint> &pts)
    -> std::vector<typename Spectrogram<T>::DataPoint> {
  return pts;
}

template <std::floating_point T>
auto Spectrogram<T>::generate_hashes_naive(
    std::vector<typename Spectrogram<T>::DataPoint> &pivots,
    std::vector<typename Spectrogram<T>::DataPoint> &localmaxima,
    size_t box_height, size_t box_width, size_t box_displacement)
    -> std::vector<std::pair<uint32_t, size_t>> {
  std::vector<std::pair<uint32_t, size_t>> hashes;
  for (const auto &pivot : pivots) {
    size_t box_xmin = pivot.time + box_displacement;
    size_t box_xmax = box_xmin + box_width;
    size_t box_ymin = pivot.hertz - (box_height / 2);
    size_t box_ymax = box_ymin + box_height;
    for (const auto &max_pt : localmaxima) {
      if ((max_pt.time >= box_xmin && max_pt.time <= box_xmax) &&
          (max_pt.hertz >= box_ymin && max_pt.hertz <= box_ymax)) {
        size_t delta_t = pivot.time < max_pt.time ? max_pt.time - pivot.time
                                                  : pivot.time - max_pt.time;
        uint32_t hash = assemble_hash(pivot.hertz, max_pt.hertz,
                                      static_cast<uint32_t>(delta_t));
        hashes.push_back(std::make_pair(hash, pivot.time));
      }
    }
  }
  return hashes;
}

template <std::floating_point T>
auto Spectrogram<T>::get_hashes() -> std::vector<std::pair<uint32_t, size_t>> {

  auto pivots = select_pivots_naive(m_features);
  return generate_hashes_naive(pivots, m_features, 200,150, 5);
}

// Explicit instantiation

template class Spectrogram<float>;
template class Spectrogram<double>;
