#include "include/save_audio.hpp"
#include "AudioFile/AudioFile.h"

static AudioFile<double>::AudioBuffer buffer_from(const int16_t *sound, size_t len, uint8_t channels, int16_t divisor)
{
	AudioFile<double>::AudioBuffer result(channels);
	for (size_t i = 0; i < result.size(); ++i)
	{
		result[i].resize(len / channels);
		for (size_t j = 0; j < result[i].size(); ++j)
		{
			result[i][j] = double(sound[j * result.size() + i]) / divisor;
		}
	}
	return std::move(result);
}

void save_audio(const std::string &path, const std::vector<int16_t> &sound, int freq, uint8_t channels, int16_t divisor, uint8_t bit_depth)
{
	AudioFile<double> file;
	auto buf = buffer_from(sound.data(), sound.size() / 2, channels, divisor);
	file.setAudioBuffer(buf);
	file.setSampleRate(freq);
	file.setBitDepth(bit_depth);
	file.save(path);
}
