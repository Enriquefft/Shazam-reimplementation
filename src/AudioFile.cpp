#include "AudioFile.hpp"
#include <stdexcept>
#include <algorithm>

#include <fstream>
#include <sstream>
#include <iostream>

/// @brief Construir audio desde un archivo usando libsndfile
Audio::Audio(const std::string &path) {
  SF_INFO sound_file_info;
  SNDFILE *sound_file = sf_open(path.c_str(), SFM_READ, &sound_file_info);
  if (sound_file == nullptr) {
    throw std::invalid_argument("Could not read file: " + path);
  }

  sf_count_t nsamples = sound_file_info.frames; // on single channel its same
  m_rate = sound_file_info.samplerate;

  m_samples.resize(static_cast<ulong>(nsamples));

  // librosa.read()
  sf_count_t nread = sf_read_short(sound_file, m_samples.data(), nsamples);

  if (nread != nsamples) {
    sf_close(sound_file);
    throw std::runtime_error("Read " + std::to_string(nread) +
                             " samples, expected " + std::to_string(nsamples));
  }
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
  return maxima_GTN_algorithm(30,0.5f);
}

std::vector<DataPoint> Spectrogram::maxima_MINLIST_algorithm(int neigh)
{
  size_t sp_x, sp_y;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0? m_spectrogram[0].size():0;

  std::vector<DataPoint> dat;

  for (uint i = 0; i < sp_x; i++) {
      for (uint j = 0; j < sp_y; j++) {
          INTENSITY_T maxfiltered = max_in_neigh(sp_x,sp_y,i,j,neigh);

          if (maxfiltered == m_spectrogram[i][j])
            dat.push_back(DataPoint{(int)i,(int)j,m_spectrogram[i][j]});
      }
  }

  return dat;
}

INTENSITY_T Spectrogram::max_in_neigh(size_t X, size_t Y, uint x, uint y, int n)
{
    // clip the overhangs of the neighbourhood
    uint xlo = (uint)std::max(0,(int)x-n),
       xhi=(uint)std::min(X,(size_t)x+n);

    uint ylo = (uint)std::max(0,(int)y-n),
       yhi=(uint)std::min(Y,(size_t)y+n);

    INTENSITY_T t = m_spectrogram[xlo][ylo];
    for (uint i = xlo;i<xhi;i++)
    {
      for (uint j = ylo;j<yhi;j++)
      {
        if (m_spectrogram[i][j] > t) t = m_spectrogram[i][j];
      }
    }
    return t;
}

std::vector<DataPoint> Spectrogram::maxima_GTN_algorithm(int neighbourhood, float thrsh)
{
  size_t sp_x, sp_y;
  // get dimensions of spectrogram
  sp_x = m_spectrogram.size();
  sp_y = sp_x > 0? m_spectrogram[0].size():0;

  std::vector<DataPoint> dat;

  // find mean loudness of the spectrogram
  float avg_loudness = 0;
  size_t num_elements = sp_x * sp_y;
  for (uint i = 0; i < sp_x; i++) {
      for (uint j = 0; j < sp_y; j++) {
          avg_loudness = ((float)num_elements / (float)(num_elements + 1)) * avg_loudness
                        + (m_spectrogram[i][j] / (float)(num_elements + 1));
      }
  }

  for (uint i = 0; i<sp_x;i++)
    for (uint j = 0; j<sp_y;j++)
    {
      // detect candidate local maxima
      if (is_max_in_neigh(sp_x,sp_y,i,j,neighbourhood,avg_loudness*thrsh)) 
        dat.push_back(DataPoint{(int)i,(int)j,m_spectrogram[i][j]});
    }
  return dat;
}

bool Spectrogram::is_max_in_neigh(size_t X, size_t Y, uint x, uint y, int n,float thrsh)
{
    // clip the overhangs of the neighbourhood
    uint xlo = (uint)std::max(0,(int)x-n),
       xhi=(uint)std::min(X,(size_t)x+n);

    uint ylo = (uint)std::max(0,(int)y-n),
       yhi=(uint)std::min(Y,(size_t)y+n);

    for (uint i = xlo;i<xhi;i++)
    {
      for (uint j = ylo;j<yhi;j++)
      {   
        //  is not the same point  
        if (x == i && y == j) continue;
        if (std::max(0.0f,m_spectrogram[i][j]-thrsh) + __FLT_EPSILON__ >= std::max(0.0f,m_spectrogram[x][y]-thrsh)) return false;
      }
    }
    return true;
}