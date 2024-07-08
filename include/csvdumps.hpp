#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <string>
#include <concepts>
#include <fstream>
#include <cerrno>

template<std::floating_point T>
void csvWriteSpectrogram(const Spectrogram<T>& spec,std::string to_write)
{
	auto sp = spec.get_spectrogram();
	// assume spectrogram is well defined, hence it has a 0th element
	size_t sx,sy;
	sx = sp.size();
	sy = sp.at(0).size();
	std::ofstream f;
	f.open(to_write,std::fstream::out | std::fstream::trunc);

	// check if open ok, if not show why

	//dump the spectrogram data to a csv
	f << sx << ',' << sy << '\n';
	for (size_t i = 0; i<sx;i++)
	{
		f << sp[i][0];
		for (size_t j = 1; j<sy;j++)
			f << ',' << sp[i][j];
		f << '\n';
	}

	f.close();
}

template<std::floating_point T>
void csvWriteSpectrogram(const std::vector<std::vector<T>>& sp,std::string to_write)
{
	// assume spectrogram is well defined, hence it has a 0th element
	size_t sx,sy;
	sx = sp.size();
	sy = sp.at(0).size();
	std::ofstream f;
	f.open(to_write,std::fstream::out | std::fstream::trunc);

	//dump the spectrogram data to a csv
	f << sx << ',' << sy << '\n';
	for (size_t i = 0; i<sx;i++)
	{
		f << sp[i][0];
		for (size_t j = 1; j<sy;j++)
			f << ',' << sp[i][j];
		f << '\n';
	}

	f.close();
}

template<typename T>
void csvWriteLocalMaxima(const std::vector<T>& pts,std::string to_write)
{

	std::ofstream f;
	f.open(to_write,std::fstream::out | std::fstream::trunc);


	for (const T& i:pts)
	{
		f << i.time << ',' << i.hertz << ',' << i.intensity << '\n';
	}

	f.close();
}


void csvWriteHashes(std::vector<std::pair<uint32_t,size_t>>& hashes,std::string to_write)
{

	std::ofstream f;
	f.open(to_write,std::fstream::out | std::fstream::trunc);
	
	for (const auto& i: hashes)
	{
		f << i.first << ',' << i.second << '\n';
	}

	f.close();
}