#pragma once

#include "RecursiveMutex.hpp"

NAMESPACE_SOUP
{
	class Mutex
	{
	private:
		RecursiveMutex mutex;
#if !SOUP_WINDOWS
		bool locked = false;
#endif

	public:
		void lock()
		{
			mutex.lock();
#if SOUP_WINDOWS
			SOUP_IF_UNLIKELY (mutex.cs.RecursionCount != 1)
#else
			SOUP_IF_UNLIKELY (locked)
#endif
			{
				mutex.unlock();
				soup::throwAssertionFailed("Attempt to obtain recursive lock on non-recursive mutex");
			}
#if !SOUP_WINDOWS
			locked = true;
#endif
		}

		[[nodiscard]] bool tryLock()
		{
			if (mutex.tryLock())
			{
#if SOUP_WINDOWS
				SOUP_IF_UNLIKELY (mutex.cs.RecursionCount != 1)
#else
				SOUP_IF_UNLIKELY (locked)
#endif
				{
					mutex.unlock();
					soup::throwAssertionFailed("Attempt to obtain recursive lock on non-recursive mutex");
				}
#if !SOUP_WINDOWS
				locked = true;
#endif
				return true;
			}
			return false;
		}

		void unlock() noexcept
		{
#if !SOUP_WINDOWS
			locked = false;
#endif
			mutex.unlock();
		}
	};
}
