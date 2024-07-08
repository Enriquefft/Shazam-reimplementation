#include "Spectrogram.hpp"
#include <algorithm>
#include <concepts>
#include <deque>
#include <sndfile.h>

using std::floating_point;
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
inline auto Spectrogram<T>::is_max_in_neigh(size_t x_max, size_t y_max,
                                            size_t x, size_t y, int n,
                                            intensity_t thrsh,
                                            const spdata_t &sp) -> bool {
  // clip the overhangs of the neighbourhood
  uint xlo = static_cast<uint>(std::max(0, static_cast<int>(x) - n));
  uint xhi = static_cast<uint>(
      std::min(x_max, static_cast<size_t>(x) + static_cast<size_t>(n)));

  uint ylo = static_cast<uint>(std::max(0, static_cast<int>(y) - n));
  uint yhi = static_cast<uint>(
      std::min(y_max, static_cast<size_t>(y) + static_cast<size_t>(n)));

  for (uint i = xlo; i < xhi; i++) {
    for (uint j = ylo; j < yhi; j++) {
      //  is not the same point
      if (x == i && y == j) {
        continue;
      }
      if (std::max(T{}, sp[i][j] - thrsh) >=
          std::max(T{}, sp[x][y] - thrsh)) {
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
  return (maxd - spd) <= __FLT_EPSILON__ && spd != 0;
}

template <floating_point T>
inline auto Spectrogram<T>::peak_filter_minlist_gtn(
    const size_t &x, const size_t &y, const spdata_t &mf, const spdata_t &sp,
    const size_t &x_max, const size_t &y_max, const int &neigh,
    const intensity_t &thresh) -> bool {
  bool is_candidate = ((mf[x][y] - sp[x][y]) <= __FLT_EPSILON__) && sp[x][y] != 0;

  if (is_candidate) {
    return is_max_in_neigh(x_max, y_max, x, y, neigh, thresh, sp);
  }
  return false;
}

template <floating_point T>
auto Spectrogram<T>::max_in_neigh(size_t x_max, size_t y_max, uint x, uint y,
                                  int n, const spdata_t &sp) -> intensity_t {
  // clip the overhangs of the neighbourhood
  uint xlo = static_cast<uint>(std::max(0, static_cast<int>(x) - n));
  uint xhi = static_cast<uint>(
      std::min(x_max, static_cast<size_t>(x) + static_cast<size_t>(n)));

  uint ylo = static_cast<uint>(std::max(0, static_cast<int>(y) - n));
  uint yhi = static_cast<uint>(
      std::min(y_max, static_cast<size_t>(y) + static_cast<size_t>(n)));

  intensity_t t = sp[xlo][ylo];
  for (uint i = xlo; i < xhi; i++) {
    for (uint j = ylo; j < yhi; j++) {
      if (sp[i][j] > t) {
        t = sp[i][j];
      }
    }
  }
  return t;
}

template <floating_point T>
void Spectrogram<T>::maxfilter_x(spdata_t &maxfiltered_spectrogram,
                                 const spdata_t &spectrogram, size_t sp_x,
                                 size_t sp_y, int neigh) {
  std::deque<size_t> wedge;
  size_t w_size = 2 * static_cast<size_t>(neigh) + 1UL;

  for (size_t x = 0; x < sp_x; ++x) {
    size_t y_start = 0;
    size_t y_read = 0;
    size_t y_write = 0;

    for (; y_read < static_cast<size_t>(neigh) && y_read < sp_y; ++y_read) {
      while (!wedge.empty() &&
             spectrogram[x][wedge.back()] <= spectrogram[x][y_read]) {
        wedge.pop_back();
      }
      wedge.push_back(y_read);
    }

    for (; y_read < sp_y; ++y_read, ++y_write) {
      while (!wedge.empty() &&
             spectrogram[x][wedge.back()] <= spectrogram[x][y_read]) {
        wedge.pop_back();
      }
      wedge.push_back(y_read);
      maxfiltered_spectrogram[x][y_write] = spectrogram[x][wedge.front()];
      if (y_read + 1 >= w_size) {
        if (wedge.front() == y_start) {
          wedge.pop_front();
        }
        ++y_start;
      }
    }

    for (; y_write < sp_y; ++y_write) {
      maxfiltered_spectrogram[x][y_write] = spectrogram[x][wedge.front()];
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

  for (size_t y = 0; y < sp_y; ++y) {
    size_t x_start = 0;
    size_t x_read = 0;
    size_t x_write = 0;

    // Initialize wedge for the first window
    for (; x_read < w_size && x_read < sp_x; ++x_read) {
      while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][y] <=
                                   maxfiltered_spectrogram[x_read][y]) {
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
      while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][y] <=
                                   maxfiltered_spectrogram[x_read][y]) {
        wedge.pop_back();
      }
      wedge.push_back(x_read);

      // Update the maxfiltered_spectrogram
      maxfiltered_spectrogram[x_write][y] =
          maxfiltered_spectrogram[wedge.front()][y];
    }

    // Finalize the remaining columns
    for (; x_write < sp_x; ++x_write) {
      // Remove elements outside the window
      while (!wedge.empty() && wedge.front() <= x_write - w_size) {
        wedge.pop_front();
      }
      
      // Update the maxfiltered_spectrogram
      maxfiltered_spectrogram[x_write][y] =
          maxfiltered_spectrogram[wedge.front()][y];
    }

    // Clear wedge for the next row
    wedge.clear();
  }
}


constexpr auto MAX_FILTER = 150;
constexpr auto GTN_WINDOW_SIZE = 30;
constexpr float MAXIMA_THRESHOLD = 0.9F;

template <floating_point T>
auto Spectrogram<T>::get_local_maximums() -> std::vector<DataPoint> {
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
  spdata_t d(sp_x,spcol_t(sp_y));

  CritSet_t dat;

  for (uint i = 0; i < sp_x; i++) {
    for (uint j = 0; j < sp_y; j++) {
      intensity_t maxfiltered =
          max_in_neigh(sp_x, sp_y, i, j, neigh, m_spectrogram);
          d[i][j] = maxfiltered;

      if (peak_filter_minlist(maxfiltered, m_spectrogram[i][j])) {
        dat.push_back(DataPoint{static_cast<hertz_t>(i), static_cast<time_t>(j),
                                m_spectrogram[i][j]});
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

  for (size_t i = 0; i < sp_x; i++) {
    for (size_t j = 0; j < sp_y; j++) {
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

  for (size_t i = 0; i < sp_x; i++) {
    for (size_t j = 0; j < sp_y; j++) {
      if (peak_filter_minlist_gtn(i, j, maxf_sp, m_spectrogram, sp_x, sp_y,
                                  gtn_s, avg_loudness * thresh)) {
        dat.push_back(DataPoint{i, j, m_spectrogram[i][j]});
      }
    }
  }

  return dat; // Return the result as needed
}

template <floating_point T>
auto Spectrogram<T>::maxima_gtn_algorithm(int neighbourhood, float thrsh)
    -> CritSet_t {
  size_t sp_x = 0;
  size_t sp_y = 0;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;

  CritSet_t dat;

  // find mean loudness of the spectrogram
  double avg_loudness_d = 0;
  size_t num_elements = sp_x * sp_y;
  for (uint i = 0; i < sp_x; i++) {
    for (uint j = 0; j < sp_y; j++) {
      avg_loudness_d =
          (static_cast<double>(num_elements) /
           static_cast<double>(num_elements + 1)) *
              avg_loudness_d +
          (m_spectrogram[i][j] / static_cast<double>(num_elements + 1));
    }
  }
  auto avg_loudness = static_cast<T>(avg_loudness_d);
  for (uint i = 0; i < sp_x; i++) {
    for (uint j = 0; j < sp_y; j++) {
      // detect candidate local maxima
      if (is_max_in_neigh(sp_x, sp_y, i, j, neighbourhood, avg_loudness * thrsh,
                          m_spectrogram)) {
        dat.push_back(DataPoint{static_cast<hertz_t>(i), static_cast<time_t>(j),
                                m_spectrogram[i][j]});
      }
    }
  }
  return dat;
}

///////////////////////////////////// HASHING ////////////////////////////////////////

uint32_t assemble_hash(size_t h1, size_t h2, size_t deltaT)
{
  // this has last 10 bits on
  const uint16_t mask = 0x3FF;

  h1 = (h1 & mask) << 20;
  h2 = (h2 & mask) << 10;
  deltaT = deltaT & mask;
  return h1 | h2 | deltaT;
}

template <std::floating_point T> 
std::vector<typename Spectrogram<T>::DataPoint> Spectrogram<T>::select_pivots_naive(
    const std::vector<typename Spectrogram<T>::DataPoint>& pts)
{
  return pts;
}

template <std::floating_point T>
std::vector<std::pair<uint32_t,size_t>> Spectrogram<T>::generate_hashes_naive(
  std::vector<typename Spectrogram<T>::DataPoint>& pivots,
  std::vector<typename Spectrogram<T>::DataPoint>& localmaxima,
  size_t boxHeight, size_t boxWidth, size_t boxDisplacement
)
{
  std::vector<std::pair<uint32_t,size_t>> hashes;
  for (const auto& i:pivots)
  {
    size_t boxXmax, boxXmin, boxYmax, boxYmin;
    boxXmin = i.time + boxDisplacement;
    boxYmin = i.hertz - (boxHeight/2);
    boxXmax = boxXmin + boxWidth;
    boxYmax = boxYmin + boxHeight;
    for (const auto& j:localmaxima)
    {
      if ((j.time >= boxXmin && j.time <= boxXmax) && 
          (j.hertz >= boxYmin && j.hertz <= boxYmax))
          {
            size_t deltaT = i.time < j.time? j.time - i.time:i.time - j.time;
            uint32_t hash = assemble_hash(i.hertz,j.hertz,deltaT);
            hashes.push_back(std::make_pair(hash,i.time));
          }
    }
  }
  return hashes;
}

template <std::floating_point T> 
std::vector<std::pair<uint32_t,size_t>> Spectrogram<T>::get_hashes()
{
  
  auto pivots = select_pivots_naive(m_features);
  return generate_hashes_naive(pivots,m_features, 200, 500,30);

}

// Explicit instantiation

template class Spectrogram<float>;
template class Spectrogram<double>;
