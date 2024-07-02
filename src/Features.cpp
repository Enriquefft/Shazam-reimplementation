#include "Spectrogram.hpp"
#include <algorithm>
#include <deque>
#include <concepts>
#include <sndfile.h>
#include <stdexcept>


using std::floating_point;
using std::pair;
using std::vector;

template <typename T> using matrix_t = vector<vector<T>>;

template <floating_point T> 
size_t Spectrogram<T>::getX() {
  // get dimensions of spectrogram
  return m_spectrogram.size();
}

template <floating_point T> 
size_t Spectrogram<T>::getY() {
  size_t sp_x = m_spectrogram.size();
  return sp_x > 0 ? m_spectrogram[0].size() : 0;
}

template <floating_point T>
inline bool Spectrogram<T>::is_max_in_neigh(size_t X, size_t Y, size_t x,
                                            size_t y, int n, intensity_t thrsh,
                                            const spdata_t &sp) {
  // clip the overhangs of the neighbourhood
  uint xlo = static_cast<uint>(std::max(0, static_cast<int>(x) - n)),
       xhi = static_cast<uint>(
           std::min(X, static_cast<size_t>(x) + static_cast<size_t>(n)));

  uint ylo = static_cast<uint>(std::max(0, static_cast<int>(y) - n)),
       yhi = static_cast<uint>(
           std::min(Y, static_cast<size_t>(y) + static_cast<size_t>(n)));

  for (uint i = xlo; i < xhi; i++) {
    for (uint j = ylo; j < yhi; j++) {
      //  is not the same point
      if (x == i && y == j)
        continue;
      if (std::max(T(0), sp[i][j] - thrsh) + __FLT_EPSILON__ >=
          std::max(T(0), sp[x][y] - thrsh))
        return false;
    }
  }
  return true;
}

template <floating_point T>
inline bool Spectrogram<T>::peak_filter_MINLIST(const intensity_t &maxd,
                                                const intensity_t &spd) {
  return maxd == spd && spd != 0;
}

template <floating_point T>
inline bool Spectrogram<T>::peak_filter_MINLIST_GTN(
    const size_t &x, const size_t &y, const spdata_t &mf, const spdata_t &sp,
    const size_t &X, const size_t &Y, const int neigh,
    const intensity_t thresh) {
  bool is_candidate = mf[x][y] == sp[x][y] && sp[x][y] != 0;

  if (is_candidate) {
    if (is_max_in_neigh(X, Y, x, y, neigh, thresh, sp))
      return true;
    return false;
  }
  return false;
}

template <floating_point T>
auto Spectrogram<T>::max_in_neigh(size_t X, size_t Y, uint x, uint y, int n,
                                  const spdata_t &sp) -> intensity_t {
  // clip the overhangs of the neighbourhood
  uint xlo = static_cast<uint>(std::max(0, static_cast<int>(x) - n)),
       xhi = static_cast<uint>(std::min(X, (size_t)x + (size_t)n));

  uint ylo = static_cast<uint>(std::max(0, static_cast<int>(y) - n)),
       yhi = static_cast<uint>(std::min(Y, (size_t)y + (size_t)n));

  intensity_t t = sp[xlo][ylo];
  for (uint i = xlo; i < xhi; i++) {
    for (uint j = ylo; j < yhi; j++) {
      if (sp[i][j] > t)
        t = sp[i][j];
    }
  }
  return t;
}

template <floating_point T>
void Spectrogram<T>::maxfilterX(spdata_t &maxfiltered_spectrogram,
                                const spdata_t &spectrogram, size_t sp_x,
                                size_t sp_y, int neigh) {
  std::deque<size_t> wedge;
  size_t w_size = 2 * (size_t)neigh + 1ul;

  for (size_t x = 0; x < sp_x; ++x) {
    size_t y_start = 0, y_read = 0, y_write = 0;

    for (; y_read < static_cast<size_t>(neigh) && y_read < sp_y; ++y_read) {
      while (!wedge.empty() &&
             spectrogram[x][wedge.back()] <= spectrogram[x][y_read])
        wedge.pop_back();
      wedge.push_back(y_read);
    }

    for (; y_read < sp_y; ++y_read, ++y_write) {
      while (!wedge.empty() &&
             spectrogram[x][wedge.back()] <= spectrogram[x][y_read])
        wedge.pop_back();
      wedge.push_back(y_read);
      maxfiltered_spectrogram[x][y_write] = spectrogram[x][wedge.front()];
      if (y_read + 1 >= w_size) {
        if (wedge.front() == y_start)
          wedge.pop_front();
        ++y_start;
      }
    }

    for (; y_write < sp_y; ++y_write) {
      maxfiltered_spectrogram[x][y_write] = spectrogram[x][wedge.front()];
      if (wedge.front() == y_start)
        wedge.pop_front();
      ++y_start;
    }
    wedge.clear();
  }
}

template <floating_point T>
void Spectrogram<T>::maxfilterY(spdata_t &maxfiltered_spectrogram, size_t sp_x,
                                size_t sp_y, int neigh) {
  std::deque<size_t> wedge;
  size_t w_size = 2 * (size_t)neigh + 1ul;

  for (size_t y = 0; y < sp_y; ++y) {
    size_t x_start = 0, x_read = 0, x_write = 0;

    for (; x_read < static_cast<size_t>(neigh) && x_read < sp_x; ++x_read) {
      while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][y] <=
                                   maxfiltered_spectrogram[x_read][y])
        wedge.pop_back();
      wedge.push_back(x_read);
    }

    for (; x_read < sp_x; ++x_read, ++x_write) {
      while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][y] <=
                                   maxfiltered_spectrogram[x_read][y])
        wedge.pop_back();
      wedge.push_back(x_read);
      maxfiltered_spectrogram[x_write][y] =
          maxfiltered_spectrogram[wedge.front()][y];
      if (x_read + 1 >= w_size) {
        if (wedge.front() == x_start)
          wedge.pop_front();
        ++x_start;
      }
    }

    for (; x_write < sp_x; ++x_write) {
      maxfiltered_spectrogram[x_write][y] =
          maxfiltered_spectrogram[wedge.front()][y];
      if (wedge.front() == x_start)
        wedge.pop_front();
      ++x_start;
    }
    wedge.clear();
  }
}


template <floating_point T>
auto Spectrogram<T>::get_local_maximums() -> std::vector<DataPoint> {
  // this is an API function that calls -some- algorithm that returns the local
  // maxima. the idea is to allow hyperparameter tuning that was not defined in
  // the API

  // return maxima_GTN_algorithm(30,0.5f);
  return maxima_MINLISTGCN_algorithm(100, 30, 0.7f);
}

template <floating_point T>
auto Spectrogram<T>::maxima_MINLIST_algorithm(int neigh) -> CritSet_t {
  size_t sp_x, sp_y;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;

  CritSet_t dat;

  for (uint i = 0; i < sp_x; i++) {
    for (uint j = 0; j < sp_y; j++) {
      intensity_t maxfiltered =
          max_in_neigh(sp_x, sp_y, i, j, neigh, m_spectrogram);

      if (peak_filter_MINLIST(maxfiltered, m_spectrogram[i][j]))
        dat.push_back(DataPoint{(hertz_t)i, (time_t)j, m_spectrogram[i][j]});
    }
  }

  return dat;
}

// Original function modified to use the above helper functions
template <floating_point T>
auto Spectrogram<T>::maxima_MINLIST_algorithm_optimized(int neigh)
    -> CritSet_t {

  size_t sp_x = m_spectrogram.size();
  size_t sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;
  spdata_t maxf_sp(sp_x, spcol_t(sp_y));

  // Apply max filters (the way they are implemented forces this order :/)
  maxfilterX(maxf_sp, m_spectrogram, sp_x, sp_y, neigh);
  maxfilterY(maxf_sp, sp_x, sp_y, neigh);

  CritSet_t dat;

  for (size_t i = 0; i < sp_x; i++) {
    for (size_t j = 0; j < sp_y; j++) {
      if (peak_filter_MINLIST(maxf_sp[i][j], m_spectrogram[i][j]))
        dat.push_back(DataPoint{(hertz_t)i, (time_t)j, m_spectrogram[i][j]});
    }
  }

  return dat; // Return the result as needed
}

template <floating_point T>
auto Spectrogram<T>::maxima_MINLISTGCN_algorithm(int maxfilter_s, int gtn_s,
                                                 intensity_t thresh)
    -> CritSet_t {

  size_t sp_x = m_spectrogram.size();
  size_t sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;
  spdata_t maxf_sp(sp_x, spcol_t(sp_y));

  // find mean loudness of the spectrogram
  double avg_loudness_d = 0;
  size_t num_elements = sp_x * sp_y;
  for (uint i = 0; i < sp_x; i++) {
    for (uint j = 0; j < sp_y; j++) {
      avg_loudness_d =
          ((double)num_elements / (double)(num_elements + 1)) * avg_loudness_d +
          (static_cast<double>(m_spectrogram[i][j]) /
           (double)(num_elements + 1));
    }
  }
  intensity_t avg_loudness = static_cast<intensity_t>(avg_loudness_d);
  // Apply max filters (the way they are implemented forces this order :/)
  maxfilterX(maxf_sp, m_spectrogram, sp_x, sp_y, maxfilter_s);
  maxfilterY(maxf_sp, sp_x, sp_y, maxfilter_s);

  CritSet_t dat;

  for (size_t i = 0; i < sp_x; i++) {
    for (size_t j = 0; j < sp_y; j++) {
      if (peak_filter_MINLIST_GTN(i, j, maxf_sp, m_spectrogram, sp_x, sp_y,
                                  gtn_s, avg_loudness * thresh))
        dat.push_back(DataPoint{(hertz_t)i, (time_t)j, m_spectrogram[i][j]});
    }
  }

  return dat; // Return the result as needed
}

template <floating_point T>
auto Spectrogram<T>::maxima_GTN_algorithm(int neighbourhood, float thrsh)
    -> CritSet_t {
  size_t sp_x, sp_y;
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
          ((double)num_elements / (double)(num_elements + 1)) * avg_loudness_d +
          (m_spectrogram[i][j] / (double)(num_elements + 1));
    }
  }
  intensity_t avg_loudness = (intensity_t)avg_loudness_d;
  for (uint i = 0; i < sp_x; i++)
    for (uint j = 0; j < sp_y; j++) {
      // detect candidate local maxima
      if (is_max_in_neigh(sp_x, sp_y, i, j, neighbourhood, avg_loudness * thrsh,
                          m_spectrogram))
        dat.push_back(DataPoint{(hertz_t)i, (time_t)j, m_spectrogram[i][j]});
    }
  return dat;
}

// Explicit instantiation
template class Spectrogram<float>;
template class Spectrogram<double>;