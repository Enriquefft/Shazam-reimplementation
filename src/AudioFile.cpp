#include "AudioFile.hpp"
#include <filesystem>
#include <sndfile.h>
#include <stdexcept>
#include <algorithm>
#include <deque>

#include <fstream>
#include <sstream>
#include <iostream>

using spdata_t = std::vector<std::vector<INTENSITY_T>>;
using spcol_t = std::vector<INTENSITY_T>;
using CritSet_t = std::vector<DataPoint>;

size_t Spectrogram::getX()
{
  // get dimensions of spectrogram
  return m_spectrogram.size();
}
size_t Spectrogram::getY()
{
    size_t sp_x = m_spectrogram.size();
    return sp_x > 0 ? m_spectrogram[0].size() : 0;
}

inline bool is_max_in_neigh(size_t X, size_t Y, size_t x, size_t y, int n,INTENSITY_T thrsh,const spdata_t& sp)
{
    // clip the overhangs of the neighbourhood
    uint xlo = (uint)std::max(0,(int)x-n),
       xhi=(uint)std::min(X,(size_t)x+(size_t)n);

    uint ylo = (uint)std::max(0,(int)y-n),
       yhi=(uint)std::min(Y,(size_t)y+(size_t)n);

    for (uint i = xlo;i<xhi;i++)
    {
      for (uint j = ylo;j<yhi;j++)
      {
        //  is not the same point
        if (x == i && y == j) continue;
        if (std::max(0.0f,sp[i][j]-thrsh) + __FLT_EPSILON__ >= std::max(0.0f,sp[x][y]-thrsh)) return false;
      }
    }
    return true;
}

inline bool peak_filter_MINLIST(const INTENSITY_T& maxd, const INTENSITY_T& spd)
{
  return maxd == spd && spd != 0;
}


inline bool peak_filter_MINLIST_GTN(const size_t& x, const size_t& y,
    const spdata_t& mf, const spdata_t& sp, const size_t& X, const size_t& Y,
    const int neigh, const INTENSITY_T thresh)
{
  bool is_candidate = mf[x][y] == sp[x][y] && sp[x][y] != 0;

  if (is_candidate)
  {
    if (is_max_in_neigh(X,Y,x,y,neigh,thresh,sp)) return true;
    return false;
  }
  return false;
}


INTENSITY_T max_in_neigh(size_t X, size_t Y, uint x, uint y, int n, const spdata_t& sp)
{
    // clip the overhangs of the neighbourhood
    uint xlo = (uint)std::max(0,(int)x-n),
       xhi=(uint)std::min(X,(size_t)x+(size_t)n);

    uint ylo = (uint)std::max(0,(int)y-n),
       yhi=(uint)std::min(Y,(size_t)y+(size_t)n);

    INTENSITY_T t = sp[xlo][ylo];
    for (uint i = xlo;i<xhi;i++)
    {
      for (uint j = ylo;j<yhi;j++)
      {
        if (sp[i][j] > t) t = sp[i][j];
      }
    }
    return t;
}

void maxfilterX(spdata_t& maxfiltered_spectrogram, const spdata_t& m_spectrogram, size_t sp_x, size_t sp_y, int neigh) {
    std::deque<size_t> wedge;
    size_t w_size = 2 * neigh + 1ul;

    for (size_t x = 0; x < sp_x; ++x) {
        size_t y_start = 0, y_read = 0, y_write = 0;

        for (; y_read < static_cast<size_t>(neigh) && y_read < sp_y; ++y_read) {
            while (!wedge.empty() && m_spectrogram[x][wedge.back()] <= m_spectrogram[x][y_read])
                wedge.pop_back();
            wedge.push_back(y_read);
        }

        for (; y_read < sp_y; ++y_read, ++y_write) {
            while (!wedge.empty() && m_spectrogram[x][wedge.back()] <= m_spectrogram[x][y_read])
                wedge.pop_back();
            wedge.push_back(y_read);
            maxfiltered_spectrogram[x][y_write] = m_spectrogram[x][wedge.front()];
            if (y_read + 1 >= w_size) {
                if (wedge.front() == y_start)
                    wedge.pop_front();
                ++y_start;
            }
        }

        for (; y_write < sp_y; ++y_write) {
            maxfiltered_spectrogram[x][y_write] = m_spectrogram[x][wedge.front()];
            if (wedge.front() == y_start)
                wedge.pop_front();
            ++y_start;
        }
        wedge.clear();
    }
}

// Helper function for max filtering along the y-axis
void maxfilterY(spdata_t& maxfiltered_spectrogram, size_t sp_x, size_t sp_y, int neigh) {
    std::deque<size_t> wedge;
    size_t w_size = 2 * neigh + 1ul;

    for (size_t y = 0; y < sp_y; ++y) {
        size_t x_start = 0, x_read = 0, x_write = 0;

        for (; x_read < static_cast<size_t>(neigh) && x_read < sp_x; ++x_read) {
            while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][y] <= maxfiltered_spectrogram[x_read][y])
                wedge.pop_back();
            wedge.push_back(x_read);
        }

        for (; x_read < sp_x; ++x_read, ++x_write) {
            while (!wedge.empty() && maxfiltered_spectrogram[wedge.back()][y] <= maxfiltered_spectrogram[x_read][y])
                wedge.pop_back();
            wedge.push_back(x_read);
            maxfiltered_spectrogram[x_write][y] = maxfiltered_spectrogram[wedge.front()][y];
            if (x_read + 1 >= w_size) {
                if (wedge.front() == x_start)
                    wedge.pop_front();
                ++x_start;
            }
        }

        for (; x_write < sp_x; ++x_write) {
            maxfiltered_spectrogram[x_write][y] = maxfiltered_spectrogram[wedge.front()][y];
            if (wedge.front() == x_start)
                wedge.pop_front();
            ++x_start;
        }
        wedge.clear();
    }
}

template <std::floating_point T>
Audio<T>::Audio(const std::filesystem::path &path,
                const std::optional<float> &sample_rate) {

  if (sample_rate != std::nullopt) {
    throw std::invalid_argument("only native sample_rate is supported");
  }

  SF_INFO sound_file_info;
  SNDFILE *sound_file = sf_open(path.c_str(), SFM_READ, &sound_file_info);
  if (sound_file == nullptr) {
    throw std::invalid_argument("Could not read file: " + path.string());
  }

  sf_count_t nsamples = sound_file_info.frames; // on single channel its same
  m_sample_rate = sound_file_info.samplerate;

  m_audiodata.resize(static_cast<ulong>(nsamples));

  sf_count_t nread = 0;

  if constexpr (std::is_same_v<T, double>) {
    nread = sf_read_double(sound_file, m_audiodata.data(), nsamples);
  }
  if constexpr (std::is_same_v<T, float>) {
    nread = sf_read_float(sound_file, m_audiodata.data(), nsamples);
  }

  if (nread != nsamples) {
    sf_close(sound_file);
    throw std::runtime_error("Read " + std::to_string(nread) +
                             " samples, expected " + std::to_string(nsamples));
  }

  sf_close(sound_file);
}

/// @brief Read a spectrogram from a CSV like a monochrome image. Delete this on final integration
/// @param fname filename of the csv to read!
Spectrogram::Spectrogram(std::string fname) {
    int width, height;

    std::string s;
    std::ifstream csv(fname);
    if (!csv.is_open()) {
        std::cerr << "Error opening spectrogram.csv" << std::endl;
        return;
    }

    // Read dimensions from the first line
    getline(csv, s);
    std::stringstream dim_ss(s);
    char delimiter;
    dim_ss >> width >> delimiter >> height;

    // Resize the spectrogram matrix
    m_spectrogram.resize(height, std::vector<INTENSITY_T>(width, 0));

    // Read spectrogram data from the rest of the file
    for (int i = 0; i < height; ++i) {
        if (!getline(csv, s)) {
            std::cerr << "Error: Insufficient data in spectrogram.csv" << std::endl;
            return;
        }
        std::stringstream line_ss(s);
        for (int j = 0; j < width; ++j) {
            std::string intensity_str;
            if (!getline(line_ss, intensity_str, ',')) {
                std::cerr << "Error: Insufficient data in row " << i + 1 << std::endl;
                return;
            }
            try {
                m_spectrogram[i][j] = static_cast<INTENSITY_T>(std::stoi(intensity_str));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
                return;
            } catch (const std::out_of_range& e) {
                std::cerr << "Out of range error: " << e.what() << std::endl;
                return;
            }
        }
    }

    csv.close();
}

auto Spectrogram::get_local_maximums() -> std::vector<DataPoint>
{
  // this is an API function that calls -some- algorithm that returns the local maxima.
  // the idea is to allow hyperparameter tuning that was not defined in the API

  //return maxima_GTN_algorithm(30,0.5f);
  return maxima_MINLISTGCN_algorithm(100,30,0.7f);
}

CritSet_t Spectrogram::maxima_MINLIST_algorithm(int neigh)
{
  size_t sp_x, sp_y;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0? m_spectrogram[0].size():0;

  CritSet_t dat;

  for (uint i = 0; i < sp_x; i++) {
      for (uint j = 0; j < sp_y; j++) {
          INTENSITY_T maxfiltered = max_in_neigh(sp_x,sp_y,i,j,neigh,m_spectrogram);

          if (peak_filter_MINLIST(maxfiltered,m_spectrogram[i][j]))
            dat.push_back(DataPoint{(HERTZ_T)i,(TIME_T)j,m_spectrogram[i][j]});
      }
  }

  return dat;
}



// Original function modified to use the above helper functions
CritSet_t Spectrogram::maxima_MINLIST_algorithm_optimized(int neigh) {

    size_t sp_x = m_spectrogram.size();
    size_t sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;
    spdata_t maxf_sp(sp_x, spcol_t(sp_y));

    // Apply max filters (the way they are implemented forces this order :/)
    maxfilterX(maxf_sp, m_spectrogram, sp_x, sp_y, neigh);
    maxfilterY(maxf_sp, sp_x, sp_y, neigh);

    CritSet_t dat;

    for (size_t i=0; i<sp_x;i++)
    {
      for (size_t j=0; j<sp_y;j++)
      {
        if (peak_filter_MINLIST(maxf_sp[i][j],m_spectrogram[i][j]))
          dat.push_back(DataPoint{(int)i,(int)j,m_spectrogram[i][j]});
      }
    }

    return dat; // Return the result as needed
}

CritSet_t Spectrogram::maxima_MINLISTGCN_algorithm(int maxfilter_s,int gtn_s,INTENSITY_T thresh) {

    size_t sp_x = m_spectrogram.size();
    size_t sp_y = sp_x > 0 ? m_spectrogram[0].size() : 0;
    spdata_t maxf_sp(sp_x, spcol_t(sp_y));

  // find mean loudness of the spectrogram
  double avg_loudness_d = 0;
  size_t num_elements = sp_x * sp_y;
  for (uint i = 0; i < sp_x; i++) {
      for (uint j = 0; j < sp_y; j++) {
          avg_loudness_d = ((double)num_elements / (double)(num_elements + 1)) * avg_loudness_d
                        + (static_cast<double>(m_spectrogram[i][j]) / (double)(num_elements + 1));
      }
  }
  INTENSITY_T avg_loudness = static_cast<INTENSITY_T>(avg_loudness_d);
    // Apply max filters (the way they are implemented forces this order :/)
    maxfilterX(maxf_sp, m_spectrogram, sp_x, sp_y, maxfilter_s);
    maxfilterY(maxf_sp, sp_x, sp_y, maxfilter_s);

    CritSet_t dat;

    for (size_t i=0; i<sp_x;i++)
    {
      for (size_t j=0; j<sp_y;j++)
      {
        if (peak_filter_MINLIST_GTN(i,j,maxf_sp,m_spectrogram,sp_x,sp_y,gtn_s,avg_loudness*thresh))
          dat.push_back(DataPoint{(int)i,(int)j,m_spectrogram[i][j]});
      }
    }

    return dat; // Return the result as needed
}

CritSet_t Spectrogram::maxima_GTN_algorithm(int neighbourhood, float thrsh)
{
  size_t sp_x, sp_y;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0? m_spectrogram[0].size():0;

  CritSet_t dat;

  // find mean loudness of the spectrogram
  double avg_loudness_d = 0;
  size_t num_elements = sp_x * sp_y;
  for (uint i = 0; i < sp_x; i++) {
      for (uint j = 0; j < sp_y; j++) {
          avg_loudness_d = ((double)num_elements / (double)(num_elements + 1)) * avg_loudness_d
                        + (m_spectrogram[i][j] / (double)(num_elements + 1));
      }
  }
  INTENSITY_T avg_loudness = (INTENSITY_T)avg_loudness_d;
  for (uint i = 0; i<sp_x;i++)
    for (uint j = 0; j<sp_y;j++)
    {
      // detect candidate local maxima
      if (is_max_in_neigh(sp_x,sp_y,i,j,neighbourhood,avg_loudness*thrsh,m_spectrogram))
        dat.push_back(DataPoint{(HERTZ_T)i,(TIME_T)j,m_spectrogram[i][j]});
    }
  return dat;
}

template <std::floating_point T>
Spectrogram<T>::Spectrogram(const Audio<T> &audio)
    : m_spectrogram({}), m_features({}) {
  std::cout << "Creating spectrogram from audio " << audio.m_sample_rate
            << '\n';
}

template <std::floating_point T>
auto Spectrogram<T>::get_spectrogram() -> std::vector<std::vector<T>> {
  return m_spectrogram;
}

template <std::floating_point T>
auto Spectrogram<T>::get_local_maximums() -> std::vector<DataPoint> {
  return {};
}

template <std::floating_point T>
auto Spectrogram<T>::stft(const Audio<T> &audio, const auto &n_fft,
                          const auto &hop_length, const auto &window_length) {

  std::cout << "Calculating STFT with params: (" << audio.m_sample_rate << '\t'
            << n_fft << '\t' << hop_length << '\n'
            << window_length << '\n';
}

// Explicit instantiation

template class Audio<float>;
template class Audio<double>;
