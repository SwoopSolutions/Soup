#pragma once

#include "memAllocator.hpp"
#include "memCAllocator.hpp"

NAMESPACE_SOUP
{
	inline memAllocator g_allocator = memCAllocator();

	[[nodiscard]] inline void* malloc(size_t size) SOUP_EXCAL { return g_allocator.allocate(size); }
	[[nodiscard]] inline void* realloc(void* addr, size_t new_size) SOUP_EXCAL { return g_allocator.reallocate(addr, new_size); }
	inline void free(void* addr) noexcept { g_allocator.deallocate(addr); }
}
