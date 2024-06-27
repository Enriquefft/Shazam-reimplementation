#include "AudioFile.hpp"

// Construir audio desde un archivo usando libsndfile
Audio::Audio(std::string path)
{
	SF_INFO fi;
	SNDFILE* f = sf_open(path.c_str(),SFM_READ,&fi);
	if (f == NULL)
	{
		throw std::invalid_argument("Could not read file: " + path);
	}

	sf_count_t nsamples = fi.frames; // on single channel its same
	rate = fi.samplerate;
	samples.resize(nsamples);
	sf_count_t nread = sf_read_short(f,samples.data(),nsamples);

	if (nread != nsamples)
	{
		sf_close(f);
		throw std::runtime_error("Read " + std::to_string(nread) + 
				" samples, expected " + std::to_string(nsamples));
	}

}
