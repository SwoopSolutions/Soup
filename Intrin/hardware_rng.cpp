#include "../soup/base.hpp"
#if SOUP_X86

#include <cstdint>

#include <immintrin.h>

NAMESPACE_SOUP
{
	namespace intrin
	{
	#if defined(__GNUC__) || defined(__clang__)
		__attribute__((target("rdseed")))
	#endif
		uint16_t hardware_rng_generate16() noexcept
		{
			uint16_t res;
			while (_rdseed16_step(&res) == 0);
			return res;
		}

	#if defined(__GNUC__) || defined(__clang__)
		__attribute__((target("rdseed")))
	#endif
		uint32_t hardware_rng_generate32() noexcept
		{
			uint32_t res;
			while (_rdseed32_step(&res) == 0);
			return res;
		}

#if SOUP_BITS == 64
	#if defined(__GNUC__) || defined(__clang__)
		__attribute__((target("rdseed")))
	#endif
		uint64_t hardware_rng_generate64() noexcept
		{
			unsigned long long res;
			while (_rdseed64_step(&res) == 0);
			return res;
		}
		static_assert(sizeof(uint64_t) == sizeof(unsigned long long));
#endif
	}
}

#endif
