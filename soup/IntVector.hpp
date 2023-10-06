#pragma once

#include <cstring> // memcpy

#ifdef _DEBUG
#include "Exception.hpp"
#endif

namespace soup
{
	template <typename T>
	class IntVector
	{
	public:
		T* m_data = nullptr;
		size_t m_capacity = 0;
		size_t m_size = 0;

		explicit constexpr IntVector() noexcept = default;

		explicit IntVector(const IntVector<T>& b) noexcept
			: m_capacity(b.m_capacity), m_size(b.m_size)
		{
			if (m_capacity != 0)
			{
				m_data = (T*)malloc(m_capacity * sizeof(T));
				memcpy(m_data, b.m_data, m_size * sizeof(T));
			}
		}

		explicit IntVector(IntVector<T>&& b) noexcept
			: m_data(b.m_data), m_capacity(b.m_capacity), m_size(b.m_size)
		{
			b.m_data = nullptr;
			b.m_capacity = 0;
			b.m_size = 0;
		}

		~IntVector() noexcept
		{
			free();
		}

		void operator=(const IntVector<T>& b) noexcept
		{
			free();

			m_size = b.m_size;
			m_capacity = b.m_capacity;

			if (m_capacity != 0)
			{
				m_data = (T*)malloc(m_capacity * sizeof(T));
				memcpy(m_data, b.m_data, m_size * sizeof(T));
			}
			else
			{
				m_data = nullptr;
			}
		}

		void operator=(IntVector<T>&& b) noexcept
		{
			free();

			m_data = b.m_data;
			m_size = b.m_size;
			m_capacity = b.m_capacity;

			b.m_data = nullptr;
			b.m_capacity = 0;
			b.m_size = 0;
		}

		[[nodiscard]] constexpr size_t size() const noexcept
		{
			return m_size;
		}

		[[nodiscard]] constexpr size_t capacity() const noexcept
		{
			return m_capacity;
		}

		[[nodiscard]] SOUP_FORCEINLINE T& operator[](size_t idx) noexcept
		{
			return m_data[idx];
		}

		[[nodiscard]] SOUP_FORCEINLINE const T& operator[](size_t idx) const noexcept
		{
			return m_data[idx];
		}

		[[nodiscard]] T& at(size_t idx)
		{
#ifdef _DEBUG
			SOUP_IF_UNLIKELY(idx >= size())
			{
				throw Exception("Out of range");
			}
#endif
			return m_data[idx];
		}

		[[nodiscard]] const T& at(size_t idx) const
		{
#ifdef _DEBUG
			SOUP_IF_UNLIKELY (idx >= size())
			{
				throw Exception("Out of range");
			}
#endif
			return m_data[idx];
		}

		void emplaceZeroesFront(size_t elms) noexcept
		{
			SOUP_IF_UNLIKELY (m_size + elms > m_capacity)
			{
				// not optimal for performance, we'd memcpy twice, luckily this path is pretty cold
				makeSpaceForMoreElements();
				if (m_size + elms > m_capacity)
				{
					// this is also a possibility if elms >= 0x1000, shit will hit the fan...
				}
			}
			memmove(&m_data[elms], &m_data[0], m_size * sizeof(T));
			memset(&m_data[0], 0, elms * sizeof(T));
			m_size += elms;
		}

		void emplace_back(T val) noexcept
		{
			if (m_size == m_capacity)
			{
				makeSpaceForMoreElements();
			}
			m_data[m_size] = val;
			++m_size;
		}

		void erase(size_t idx) noexcept
		{
			if (--m_size != idx)
			{
				memcpy(&m_data[idx], &m_data[idx + 1], (m_size - idx) * sizeof(T));
			}
		}

		void eraseFirst(size_t num) noexcept
		{
			m_size -= num;
			memcpy(&m_data[0], &m_data[num], m_size * sizeof(T));
		}

		void preallocate() noexcept
		{
			if (m_capacity == 0)
			{
				m_capacity = (0x1000 / sizeof(T));
				m_data = reinterpret_cast<T*>(malloc(m_capacity * sizeof(T)));
			}
		}

	protected:
		void makeSpaceForMoreElements() noexcept
		{
			m_capacity += (0x1000 / sizeof(T));
			m_data = reinterpret_cast<T*>(m_data ? realloc(m_data, m_capacity * sizeof(T)) : malloc(m_capacity * sizeof(T)));
		}

	public:
		void clear() noexcept
		{
			m_size = 0;
		}

		void reset() noexcept
		{
			clear();
			if (m_capacity != 0)
			{
				m_capacity = 0;
				::free(m_data);
				m_data = nullptr;
			}
		}

	protected:
		void free() noexcept
		{
			if (m_capacity != 0)
			{
				::free(m_data);
			}
		}
	};
}
