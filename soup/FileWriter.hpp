#pragma once

#include "Writer.hpp"

#include <filesystem>
#include <fstream>

#include "type_traits.hpp"

NAMESPACE_SOUP
{
	class FileWriter final : public Writer
	{
	public:
		std::ofstream s;

		FileWriter(const std::string& path, bool little_endian = true)
			: Writer(little_endian), s(path, std::ios::binary)
		{
		}

#if SOUP_WINDOWS && !SOUP_CROSS_COMPILE
		FileWriter(const std::wstring& path, bool little_endian = true)
			: Writer(little_endian), s(path, std::ios::binary)
		{
		}
#endif

		template <class T, SOUP_RESTRICT(std::is_same_v<T, std::filesystem::path>)>
		FileWriter(const T& path, bool little_endian = true)
			: Writer(little_endian), s(path, std::ios::binary)
		{
		}

		~FileWriter() final = default;

		[[nodiscard]] bool isOkay() const noexcept
		{
			return s.is_open();
		}

		void throwIfFailed() const
		{
			SOUP_ASSERT(s.is_open(), "Failed to open file for writing");
		}

		bool raw(void* data, size_t size) noexcept final
		{
			SOUP_TRY
			{
				s.write(reinterpret_cast<char*>(data), size);
			}
			SOUP_CATCH_ANY
			{
				return false;
			}
			return true;
		}

		[[nodiscard]] size_t getPosition() final
		{
			return s.tellp();
		}
	};
}
