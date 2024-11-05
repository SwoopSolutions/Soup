#pragma once

#include <cstddef> // size_t
#include <memory> // destroy_at

#include "base.hpp"
#include "memory.hpp" // construct_at

NAMESPACE_SOUP
{
	using destroyer_t = void(*)(void*);

	template <typename T>
	void destroyer_impl(void* ptr)
	{
		std::destroy_at<>(reinterpret_cast<T*>(ptr));
	}

	// memBox has the polymorphism of Capture with the flat memory layout of Optional.
	// Albeit you need to know the size of the biggest object to be stored up-front.
	template <size_t Bytes>
	struct memBox
	{
		char buf[Bytes]{};
		destroyer_t destroyer = nullptr;

		~memBox()
		{
			if (destroyer)
			{
				destroyer(&buf[0]);
			}
		}

		[[nodiscard]] bool isConstructed() const noexcept
		{
			return destroyer != nullptr;
		}

		template <typename T, typename...Args>
		T* construct(Args&&...args)
		{
			static_assert(sizeof(T) <= Bytes, "T is too big for this box. Try a bigger box.");
			SOUP_ASSERT(!isConstructed());
			soup::construct_at<T, Args...>(reinterpret_cast<T*>(&buf[0]), std::forward<Args>(args)...);
			destroyer = &destroyer_impl<T>;
			return reinterpret_cast<T*>(&buf[0]);
		}

		void destroy()
		{
			SOUP_ASSERT(isConstructed());
			destroyer(&buf[0]);
			destroyer = nullptr;
		}

		void reset()
		{
			if (isConstructed())
			{
				destroy();
			}
		}

		[[nodiscard]] operator bool() const noexcept
		{
			return isConstructed();
		}

		template <typename T>
		[[nodiscard]] T& get() noexcept
		{
			return *reinterpret_cast<T*>(&buf[0]);
		}

		template <typename T>
		[[nodiscard]] const T& get() const noexcept
		{
			return *reinterpret_cast<const T*>(&buf[0]);
		}
	};
}
