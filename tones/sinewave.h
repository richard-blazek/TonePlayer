#pragma once

#include <cmath>
#include <vector>

namespace tones
{
	class SineWave
	{
	private:
		constexpr static double SinCycle = 2 * M_PI;
		int lenght;

	public:
		SineWave() = default;
		SineWave(int len) : lenght(len) {}
		int Lenght() const
		{
			return lenght;
		}
		void SetLenght(int len)
		{
			lenght = len;
		}
		template <typename T>
		std::vector<T> Build(T volume) const
		{
			std::vector<T> result(lenght);
			for (size_t i = 0; i < result.Size(); ++i)
			{
				result[i] = sin(i * SinCycle / lenght) * volume;
			}
			return std::move(result);
		}
	};
}