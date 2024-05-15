#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "fwd.hpp"
#include "base.hpp"

NAMESPACE_SOUP
{
	class Pattern
	{
	public:
		std::vector<std::optional<uint8_t>> bytes;
#if SOUP_X86 && SOUP_BITS == 64
		size_t most_unique_byte_index;
#endif

		explicit Pattern() = default;
		Pattern(const CompiletimePatternWithOptBytesBase& sig);
		Pattern(const std::string& str);
		Pattern(const char* str, size_t len);
	private:
		void computeMostUniqueByteIndex();

	public:
		[[nodiscard]] std::string toString() const SOUP_EXCAL;
	};
}
