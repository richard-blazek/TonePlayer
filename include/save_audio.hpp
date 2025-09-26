#ifndef __SAVE_AUDIO_H__
#define __SAVE_AUDIO_H__

#include <stdint.h>
#include <string>
#include <vector>

void save_audio(const std::string &path, const std::vector<int16_t> &sound, int freq, uint8_t channels, int16_t divisor, uint8_t bit_depth);

#endif
