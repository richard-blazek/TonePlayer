#include <iostream>
#include <string>
#include <fstream>
#include <SDL2/SDL.h>

#include "include/save_audio.hpp"
#include "include/sound_data.hpp"

constexpr double INTERVAL = std::pow(2.0, 1.0 / 12.0);

int sin_wave(int freq, int output_freq, int i, int volume)
{
	return sin(i * M_PI * freq / output_freq) * volume;
}

int sound_wave(int freq, int output_freq, int i, int volume)
{
	return sin_wave(freq, output_freq, i, volume / 2) + sin_wave(freq * 2, output_freq, i + output_freq / 2, volume / 4) + sin_wave(freq * 3, output_freq, i, volume / 8) + sin_wave(freq * 5, output_freq, i + output_freq / 2, volume / 16) + sin_wave(freq * 8, output_freq, i, volume / 32) + sin_wave(freq * 13, output_freq, i + output_freq / 2, volume / 32);
}

int calc_volume(int tone_lenght, int i, int volume)
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

	void draw(SDL_Renderer *rend)
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

	SoundData to_sound_data(int output_freq, uint8_t channels, int16_t volume)
	{
		SoundData sound(length * output_freq * 2 * channels / 1000);
		for (size_t tone = 0; tone < tones.size(); ++tone)
		{
			for (size_t i = 0, len = tones[tone].len * output_freq * channels / 1000; i < len; ++i)
			{
				sound[i + tones[tone].pos * output_freq * channels / 1000] += sound_wave(tones[tone].freq, output_freq, i / channels, calc_volume(len * 2, i, volume * tones[tone].volume / 1000));
			}
		}
		for (size_t i = 1, len = sound.size(); i < len; ++i)
		{
			if (abs(sound[i] - sound[i-1]) > volume / 16)
			{
				sound[i] = sound[i-1] + volume / 16 * (sound[i] > sound[i-1] ? 1 : -1);
			}
		}
		return std::move(sound);
	}

	static Song read(std::istream &in)
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
				new_tone.freq = (tone == "rest" ? 0 : int(pow(INTERVAL, stoi(tone)) * base));
				new_tone.pos = (position == "after" ? tones.back().pos + tones.back().len : stoi(position));
				tones.push_back(new_tone);
			}
		}
		return std::move(Song(total_lenght, std::move(tones)));
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		puts("No file provided");
		return 0;
	}
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *screen = SDL_CreateWindow("Tone Player", 50, 50, 1200, 600, SDL_WINDOW_RESIZABLE);
	SDL_Renderer *rend = SDL_CreateRenderer(screen, -1, 0);
	SDL_RenderPresent(rend);

	try
	{
		std::ifstream infile(argv[1]);
		Song song = Song::read(infile);

		SoundData sound;
		SDL_AudioSpec audio{48000, AUDIO_S16, 1, 0, 4096, 0, 2 * 4096, SoundData::fill, &sound};
		sound = song.to_sound_data(audio.freq, audio.channels, 0x7fff);
		SDL_AudioDeviceID device = SDL_OpenAudioDevice(0, false, &audio, nullptr, 0);
		SDL_PauseAudioDevice(device, 0); // = play

		save_audio(std::string(argv[1]) + ".wav", sound.get_buffer(), audio.freq, audio.channels, 0x7fff, 24);

		SDL_Event event;
		while (!SDL_QuitRequested())
		{
			int screen_w = 0, screen_h = 0;
			SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

			SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
			SDL_RenderClear(rend);

			song.draw(rend);

			SDL_SetRenderDrawColor(rend, 255, 0, 255, 255);
			SDL_Rect cursor_rect{screen_w * (int)sound.get_offset() / (int)sound.size(), 0, 2, screen_h};
			SDL_RenderFillRect(rend, &cursor_rect);

			SDL_RenderPresent(rend);

			while(SDL_PollEvent(&event))
			{
				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					sound.set_offset(event.button.x * sound.size() / screen_w);
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