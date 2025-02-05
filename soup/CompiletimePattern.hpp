#pragma once

#include "CompiletimePatternBase.hpp"

#include <array>

NAMESPACE_SOUP
{
	template <size_t len>
	struct CompiletimePattern : public CompiletimePatternBase
	{
		std::array<uint8_t, len> aob{};

		constexpr CompiletimePattern(const char* sig)
		{
			size_t sig_i = 0;
			for (size_t aob_i = 0; aob_i < len; aob_i++)
			{
				const char first = sig[sig_i++];
				const char second = sig[sig_i++];
				aob[aob_i] = uint8_t(hex_to_int(first) * 0x10 + hex_to_int(second) & 0xFF);
				sig_i++;
			}
		}

		[[nodiscard]] bool operator!=(const void* addr) const
		{
			auto target = reinterpret_cast<const uint8_t*>(addr);
			for (const auto& byte : aob)
			{
				if (byte != *target)
				{
					return true;
				}
				++target;
			}
			return false;
		}

		std::vector<uint8_t> getVec() const
		{
			return std::vector<uint8_t>(aob.begin(), aob.end());
		}
	};
}
