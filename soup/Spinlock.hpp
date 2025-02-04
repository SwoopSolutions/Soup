#pragma once

#include <atomic>

#include "base.hpp"

NAMESPACE_SOUP
{
	// Not recommended to be used in production. Mutex is generally a much better option.
	class Spinlock
	{
	protected:
		std::atomic_bool locked = false;

	public:
		void lock() noexcept
		{
			bool expected;
			while (expected = false, !locked.compare_exchange_weak(expected, true));
		}

		[[nodiscard]] bool tryLock() noexcept
		{
			bool expected = false;
			return locked.compare_exchange_strong(expected, true);
		}

		void unlock() noexcept
		{
			locked = false;
		}

		// Non-atomic operations

		[[nodiscard]] bool isLocked() const noexcept
		{
			return locked;
		}
	};
}
