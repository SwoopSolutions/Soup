#include "MemoryBuffer.hpp"

#if SOUP_WINDOWS

#include "alloc.hpp"
#include "Module.hpp"

NAMESPACE_SOUP
{
	MemoryBuffer::MemoryBuffer() noexcept
		: start(nullptr), data(nullptr), size(0)
	{
	}

	MemoryBuffer::MemoryBuffer(const Module& mod, Pointer start, size_t size) SOUP_EXCAL
		: start(start), data(soup::malloc(size))
	{
		this->size = mod.externalRead(start, data, size);
	}

	MemoryBuffer::~MemoryBuffer()
	{
		release();
	}

	bool MemoryBuffer::updateRegion(const Module& mod, Pointer start, size_t size) SOUP_EXCAL
	{
		if (this->size != size)
		{
			release();
			data = soup::malloc(size);
		}
		this->start = start;
		this->size = mod.externalRead(start, data, size);
		return this->size != 0;
	}

	void MemoryBuffer::release() noexcept
	{
		if (data != nullptr)
		{
			soup::free(data);
		}
	}

	bool MemoryBuffer::covers(Pointer p, size_t size) const noexcept
	{
		return start.as<uintptr_t>() <= p.as<uintptr_t>()
			&& (p.as<uintptr_t>() + size) <= (start.as<uintptr_t>() + this->size)
			;
	}
}

#endif
