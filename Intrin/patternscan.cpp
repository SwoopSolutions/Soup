#include "../soup/base.hpp"
#if SOUP_X86 && SOUP_BITS == 64

#include <immintrin.h>

#include "../soup/bitutil.hpp"
#include "../soup/Pattern.hpp"
#include "../soup/Range.hpp"

NAMESPACE_SOUP
{
	#if defined(__GNUC__) || defined(__clang__)
		__attribute__((target("avx2")))
	#endif
	size_t Range::scanWithMultipleResultsAvx2(const Pattern& sig, Pointer buf[], size_t buflen) const noexcept
	{
		const auto data = sig.bytes.data();
		const auto length = sig.bytes.size();
		const auto match = _mm256_set1_epi8(*data[sig.most_unique_byte_index]);
		size_t accum = 0;
		for (uintptr_t i = sig.most_unique_byte_index; i < (size - length - 31); i += 32)
		{
			uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(match, _mm256_loadu_si256(base.add(i).as<const __m256i*>())));
			while (mask != 0)
			{
				const auto j = bitutil::getLeastSignificantSetBit(mask);
				if (pattern_matches(base.add(i).add(j).sub(sig.most_unique_byte_index).as<uint8_t*>(), data, length))
				{
					buf[accum++] = base.add(i).add(j).sub(sig.most_unique_byte_index);
					if (accum == buflen)
					{
						return buflen;
					}
				}
				bitutil::unsetLeastSignificantSetBit(mask);
			}
		}
		return accum;
	}

#if false
	size_t Range::scanWithMultipleResultsAvx512(const Pattern& sig, Pointer buf[], size_t buflen) const noexcept
	{
		const auto data = sig.bytes.data();
		const auto length = sig.bytes.size();
		const auto match = _mm512_set1_epi8(*data[sig.most_unique_byte_index]);
		size_t accum = 0;
		for (uintptr_t i = sig.most_unique_byte_index; i < (size - length - 63); i += 64)
		{
			uint64_t mask = _mm512_cmpeq_epi8_mask(match, _mm512_loadu_si512(base.add(i).as<const __m512i*>()));
			while (mask != 0)
			{
				const auto j = bitutil::getLeastSignificantSetBit(mask);
				if (pattern_matches(base.add(i).add(j).sub(sig.most_unique_byte_index).as<uint8_t*>(), data, length))
				{
					buf[accum++] = base.add(i).add(j).sub(sig.most_unique_byte_index);
					if (accum == buflen)
					{
						return buflen;
					}
				}
				bitutil::unsetLeastSignificantSetBit(mask);
			}
		}
		return accum;
	}
#endif
}

#endif
