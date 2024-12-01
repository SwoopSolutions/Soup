#pragma once

#define SOUP_PACKET(name) struct name : public ::soup::Packet<name>
#define SOUP_PACKET_IO(s) template <typename T> bool io(T& s)
#define SOUP_IF_ISREAD if constexpr (T::isRead())
#define SOUP_ELSEIF_ISWRITE else

#include "BufferWriter.hpp"
#include "MemoryRefReader.hpp"
#include "StringReader.hpp"
#include "StringWriter.hpp"
#include "utility.hpp" // SOUP_MOVE_RETURN

NAMESPACE_SOUP
{
	template <typename T>
	class Packet
	{
	protected:
		using i8 = int8_t;
		using i16 = int16_t;
		using i32 = int32_t;
		using i64 = int64_t;
		using u8 = uint8_t;
		using u16 = uint16_t;
		using u32 = uint32_t;
		using u64 = uint64_t;
		using u24 = u32;
		using u40 = u64;
		using u48 = u64;
		using u56 = u64;

	public:
		bool fromBinary(std::string&& bin) noexcept
		{
			StringReader r(std::move(bin), ENDIAN_BIG);
			return read(r);
		}

		[[deprecated]] bool fromBinary(std::string&& bin, Endian endian) noexcept
		{
			StringReader r(std::move(bin), endian);
			return read(r);
		}

		bool fromBinary(const std::string& bin) noexcept
		{
			MemoryRefReader r(bin, ENDIAN_BIG);
			return read(r);
		}

		[[deprecated]] bool fromBinary(const std::string& bin, Endian endian) noexcept
		{
			MemoryRefReader r(bin, endian);
			return read(r);
		}

		[[deprecated]] bool fromBinaryLE(std::string&& bin) noexcept
		{
			StringReader r(std::move(bin), ENDIAN_LITTLE);
			return read(r);
		}

		[[deprecated]] bool fromBinaryLE(const std::string& bin) noexcept
		{
			MemoryRefReader r(bin, ENDIAN_LITTLE);
			return read(r);
		}

		template <typename Reader = Reader>
		bool read(Reader& r)
		{
			return static_cast<T*>(this)->template io<Reader>(r);
		}

		[[nodiscard]] Buffer toBinary() SOUP_EXCAL
		{
			BufferWriter w(ENDIAN_BIG);
			write(w);
			SOUP_MOVE_RETURN(w.buf);
		}

		[[deprecated]] Buffer toBinary(Endian endian) SOUP_EXCAL
		{
			BufferWriter w(endian);
			write(w);
			SOUP_MOVE_RETURN(w.buf);
		}

		[[nodiscard]] std::string toBinaryString() SOUP_EXCAL
		{
			StringWriter w(ENDIAN_BIG);
			write(w);
			SOUP_MOVE_RETURN(w.data);
		}

		[[deprecated]] std::string toBinaryString(Endian endian) SOUP_EXCAL
		{
			StringWriter w(endian);
			write(w);
			SOUP_MOVE_RETURN(w.data);
		}

		[[deprecated]] std::string toBinaryStringLE()
		{
			return toBinaryString(ENDIAN_LITTLE);
		}

		template <typename Writer = Writer>
		bool write(Writer& w)
		{
			return static_cast<T*>(this)->template io<Writer>(w);
		}
	};
}
