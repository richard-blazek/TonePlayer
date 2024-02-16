#include <iostream>
#include <string>
#include <fstream>
#include <SDL2/SDL.h>
#include "AudioFile/AudioFile.h"

#include "tones/tone.h"

constexpr double Interval = std::pow(2.0, 1.0 / 12.0);

template <typename T>
struct SoundData
{
	std::vector<T> buffer;
	int pos;

	int size()
	{
		return buffer.size();
	}

	static void Fill(void *userdata, Uint8 *stream, int length)
	{
		auto &sound = *(SoundData<T> *)userdata;
		int len = std::min(length, int(sizeof(T) * sound.size() - sound.pos));
		memcpy(stream, (Uint8 *)sound.buffer.data() + sound.pos, len);
		if (len < length)
		{
			memcpy(stream + len, sound.buffer.data(), length - len);
		}
		sound.pos = (sound.pos + length) % sound.size();
	}
};

int SinWaveSound(int freq, int output_freq, int i, int volume)
{
	return sin(i * M_PI * freq / output_freq) * volume;
}

int NiceSound(int freq, int output_freq, int i, int volume)
{
	return SinWaveSound(freq, output_freq, i, volume / 2) + SinWaveSound(freq * 2, output_freq, i + output_freq / 2, volume / 4) + SinWaveSound(freq * 3, output_freq, i, volume / 8) + SinWaveSound(freq * 5, output_freq, i + output_freq / 2, volume / 16) + SinWaveSound(freq * 8, output_freq, i, volume / 32) + SinWaveSound(freq * 13, output_freq, i + output_freq / 2, volume / 32);
}

int Volume(int tone_lenght, int i, int volume)
{
	return volume * (tone_lenght - (i < tone_lenght * 2 / 3 ? 0 : (i - tone_lenght * 2 / 3) * 4 / 3)) / tone_lenght;
}

struct Tone
{
	int freq, len, pos, volume;
};

class Song
{
private:
	int length;
	std::vector<Tone> tones;

public:
	Song(int length, std::vector<Tone> tones)
		: length(length), tones(std::move(tones)) {}
	void DrawOn(SDL_Renderer *rend)
	{
		int screen_w = 0, screen_h = 0;
		SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

		for (size_t i = 0; i < tones.size(); ++i)
		{
			int height = std::log(tones[i].freq) * 200 - 1000;
			SDL_SetRenderDrawColor(rend, 0, 255, 0, 255);
			SDL_Rect rect{screen_w * tones[i].pos / length, screen_h - height, screen_w * tones[i].len / length, 2};
			SDL_RenderFillRect(rend, &rect);
		}
	}
	template <typename T>
	SoundData<T> CreateSound(int output_freq, Uint8 channels, T volume)
	{
		SoundData<T> sound{std::vector<T>(length * output_freq * 2 * channels / 1000), 0};
		for (size_t tone = 0; tone < tones.size(); ++tone)
		{
			for (size_t i = 0, len = tones[tone].len * output_freq * channels / 1000; i < len; ++i)
			{
				sound.buffer[i + tones[tone].pos * output_freq * channels / 1000] += NiceSound(tones[tone].freq, output_freq, i / channels, Volume(len * 2, i, volume * tones[tone].volume / 1000));
			}
		}
		for (size_t i = 1, len = sound.buffer.size(); i < len; ++i)
		{
			if (abs(sound.buffer[i] - sound.buffer[i - 1]) > volume / 16)
			{
				sound.buffer[i] = sound.buffer[i - 1] + volume / 16 * (sound.buffer[i] > sound.buffer[i - 1] ? 1 : -1);
			}
		}
		return std::move(sound);
	}
};

Song LoadSong(std::istream &in)
{
	std::vector<Tone> tones;
	int base = 0, total_lenght = 0;
	std::string tone, length, position, volume;
	in >> base >> total_lenght;
	while (in >> tone >> length >> volume >> position)
	{
		Tone new_tone{0, stoi(length), 0, stoi(volume)};
		if (new_tone.len > 0)
		{
			new_tone.freq = (tone == "rest" ? 0 : int(pow(Interval, stoi(tone)) * base));
			new_tone.pos = (position == "after" ? tones.back().pos + tones.back().len : stoi(position));
			tones.push_back(new_tone);
		}
	}
	return std::move(Song(total_lenght, std::move(tones)));
}

template <typename T>
AudioFile<double>::AudioBuffer AudioBufferFrom(const T *sound, size_t len, Uint8 channels, T divisor)
{
	std::vector<std::vector<double>> result(channels);
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

template <typename T>
void SaveSound(const std::string &path, const SoundData<T> &sound, int freq, Uint8 channels, T divisor, Uint8 bit_depth)
{
	AudioFile<double> out_file;
	auto buffer = AudioBufferFrom(sound.buffer.data(), sound.buffer.size() / 2, channels, divisor);
	out_file.setAudioBuffer(buffer);
	out_file.setSampleRate(freq);
	out_file.setBitDepth(bit_depth);
	out_file.save(path);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		puts("No file provided");
		return 0;
	}
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *screen = SDL_CreateWindow("Pokus", 50, 50, 1200, 600, SDL_WINDOW_RESIZABLE);
	SDL_Renderer *rend = SDL_CreateRenderer(screen, -1, 0);
	SDL_RenderPresent(rend);

	try
	{
		std::ifstream infile(argv[1]);
		Song song = LoadSong(infile);

		SoundData<Sint16> sound;
		SDL_AudioSpec audio{48000, AUDIO_S16, 1, 0, 4096, 0, 2 * 4096, SoundData<Sint16>::Fill, &sound};
		sound = song.CreateSound<Sint16>(audio.freq, audio.channels, 0x7fff);
		SDL_AudioDeviceID device = SDL_OpenAudioDevice(0, false, &audio, nullptr, 0);
		SDL_PauseAudioDevice(device, 0); // = play

		SaveSound<Sint16>(std::string(argv[1]) + ".wav", sound, audio.freq, audio.channels, 0x7fff, 24);

		SDL_Event event;
		while (!SDL_QuitRequested())
		{
			int screen_w = 0, screen_h = 0;
			SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

			SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
			SDL_RenderClear(rend);

			song.DrawOn(rend);

			SDL_SetRenderDrawColor(rend, 255, 0, 255, 255);
			SDL_Rect cursor_rect{screen_w * sound.pos / sound.size(), 0, 2, screen_h};
			SDL_RenderFillRect(rend, &cursor_rect);

			SDL_RenderPresent(rend);

			while(SDL_PollEvent(&event))
			{
				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					sound.pos = event.button.x * sound.buffer.size() / screen_w;
				}
			}
			SDL_Delay(50);
		}
	}
	catch (std::invalid_argument &exc)
	{
		puts("The opened file has an incorrect format");
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(screen);
	SDL_Quit();
	return 0;
}