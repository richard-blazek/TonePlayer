#pragma once

#include "sinewave.h"

namespace tones
{
	class Tone
	{
	private:
		constexpr static double Ratio = std::pow(2, 1.0 / 12);
		int freq = 1;

	public:
		constexpr Tone() = default;
		constexpr Tone(int frequency) : freq(frequency) {}
		constexpr int Frequency() const
		{
			return freq;
		}
		constexpr void SetFrequency(int new_freq)
		{
			freq = new_freq;
		}
		constexpr Tone Interval(int x) const
		{
			return Tone(int(freq * std::pow(Ratio, x)));
		}
		SineWave AsSineWave(int result_freq) const noexcept
		{
			return SineWave(result_freq / freq);
		}
		template <typename T, typename Iterator>
		static size_t WriteSound(const std::vector<T> &source, Iterator stream, size_t size, size_t pos)
		{
			for (size_t i = 0; i < size; ++i)
			{
				stream[i] = source[(pos + i) % source.size()];
			}
			return (pos + size) % source.size();
		}
	};
}