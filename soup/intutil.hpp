#pragma once

namespace soup
{
	struct intutil
	{
		// C++23 will add std::byteswap
		[[nodiscard]] static constexpr uint32_t invertEndianness(uint32_t val)
		{
			return val << (32 - 8)
				| ((val >> 8) & 0xFF) << (32 - 16)
				| ((val >> 16) & 0xFF) << (32 - 24)
				| ((val >> 24) & 0xFF)
				;
		}
		
		[[nodiscard]] static constexpr uint64_t invertEndianness(uint64_t val)
		{
			return val << (64 - 8)
				| ((val >> 8) & 0xFF) << (64 - 16)
				| ((val >> 16) & 0xFF) << (64 - 24)
				| ((val >> 24) & 0xFF) << (64 - 32)
				| ((val >> 32) & 0xFF) << (64 - 40)
				| ((val >> 40) & 0xFF) << (64 - 48)
				| ((val >> 48) & 0xFF) << (64 - 56)
				| ((val >> 56) & 0xFF)
				;
		}

		template <typename T>
		[[nodiscard]] static constexpr T pow(T a, T b) // b must be > 0
		{
			T res = a;
			while (--b)
			{
				res *= a;
			}
			return res;
		}
	};
}
