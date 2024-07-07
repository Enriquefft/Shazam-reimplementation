#include "AudioFile.hpp"
#include "Spectrogram.hpp"
#include <string>
#include <concepts>
#include <fstream>
#include <cerrno>

template<std::floating_point T>
void csvWriteSpectrogram(Spectrogram<T>& spec,std::string to_write)
{
	auto sp = spec.get_spectrogram();
	// assume spectrogram is well defined, hence it has a 0th element
	size_t sx,sy;
	sx = sp.size();
	sy = sp.at(0).size();
	std::ofstream f;
	f.open(to_write,std::fstream::out | std::fstream::trunc);

	// check if open ok, if not show why
	if (!f)
	{
		std::cout << std::strerror(errno) << std::endl;
		return;
	}

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
void csvWriteLocalMaxima(std::vector<T> pts,std::string to_write)
{

	std::ofstream f;
	f.open(to_write,std::fstream::out | std::fstream::trunc);

	// check if open ok, if not show why
	if (!f)
	{
		std::cout << std::strerror(errno) << std::endl;
		return;
	}

	for (const T& i:pts)
	{
		f << i.time << ',' << i.hertz << '\n';
	}

	f.close();
}