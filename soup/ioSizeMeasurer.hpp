#pragma once

#include "Writer.hpp"

NAMESPACE_SOUP
{
	class ioSizeMeasurer final : public Writer
	{
	public:
		size_t size = 0;

		ioSizeMeasurer()
			: Writer(ENDIAN_NATIVE)
		{
		}

		~ioSizeMeasurer() final = default;

		bool raw(void* data, size_t size) noexcept final
		{
			this->size += size;
			return true;
		}

		[[nodiscard]] size_t getPosition() final
		{
			return size;
		}
	};
}
