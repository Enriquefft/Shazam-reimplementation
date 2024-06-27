#include <vector>
#include <string>
#include <sndfile.h>
#include <stdexcept>

class Audio
{
public:
	std::vector<short> samples;
	uint64_t rate;
public:
	//Read a single-track audio file
	Audio(std::string path);
};