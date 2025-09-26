#ifndef __SOUND_DATA_H__
#define __SOUND_DATA_H__

#include <stdint.h>
#include <string.h>
#include <vector>

class SoundData
{
private:
	std::vector<int16_t> buffer;
	size_t offset;

public:
    SoundData();
    SoundData(size_t length);

    const std::vector<int16_t> &get_buffer() const;
	size_t get_offset() const;
	void set_offset(size_t offset);
	int16_t *data();
	size_t size() const;
    int16_t &operator[](size_t i);

	static void fill(void *userdata, uint8_t *stream, int length);
};

#endif
