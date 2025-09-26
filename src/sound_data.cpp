#include "include/sound_data.hpp"

SoundData::SoundData() : buffer(), offset(0) {}
SoundData::SoundData(size_t length) : buffer(length), offset(0) {}

const std::vector<int16_t> &SoundData::get_buffer() const
{
    return buffer;
}

size_t SoundData::get_offset() const
{
    return offset;
}

void SoundData::set_offset(size_t offset)
{
    this->offset = offset;
}

int16_t *SoundData::data()
{
    return buffer.data();
}

size_t SoundData::size() const
{
    return buffer.size();
}

int16_t &SoundData::operator[](size_t i)
{
    return buffer[i];
}

void SoundData::fill(void *userdata, uint8_t *stream, int length)
{
    SoundData &sound = *(SoundData *)userdata;

    int rest = 2 * sound.size() - sound.offset;
    if (rest < length)
    {
        memcpy(stream, (uint8_t *)sound.data() + sound.offset, rest);
        memcpy(stream + rest, (uint8_t *)sound.data(), length - rest);
    }
    else
    {
        memcpy(stream, (uint8_t *)sound.data() + sound.offset, length);
    }

    sound.offset = (sound.offset + length) % sound.size();
}
