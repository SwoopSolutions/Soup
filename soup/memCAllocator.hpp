#pragma once

#include "memAllocator.hpp"

#include "alloc.hpp"

NAMESPACE_SOUP
{
	class memCAllocator : public memAllocator
	{
	public:
		memCAllocator() noexcept
			: memAllocator(&allocateImpl, &deallocateImpl)
		{
		}

	protected:
		static void* allocateImpl(memAllocator*, size_t size) SOUP_EXCAL
		{
			return soup::malloc(size);
		}

		static void deallocateImpl(memAllocator*, void* addr) noexcept
		{
			return soup::free(addr);
		}
	};
}
