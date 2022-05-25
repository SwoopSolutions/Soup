#pragma once

#include "IoBase.hpp"

namespace soup
{
	class Writer : public IoBase<false>
	{
	public:
		using IoBase::IoBase;

	protected:
		virtual void write(const char* data, size_t size) = 0;

	public:
		bool ignore(size_t len)
		{
			uint8_t v = 0;
			while (len-- != 0)
			{
				u8(v);
			}
			return true;
		}

		bool u8(uint8_t& v) final
		{
			write((const char*)&v, sizeof(uint8_t));
			return true;
		}

		// An unsigned 64-bit integer encoded in 1..9 bytes. The most significant bit of bytes 1 to 8 is used to indicate if another byte follows.
		bool u64_dyn(const uint64_t& v)
		{
			uint64_t in = v;
			for (uint8_t i = 0; i != 8; ++i)
			{
				uint8_t cur = (in & 0x7F);
				in >>= 7;
				if (in != 0)
				{
					cur |= 0x80;
					u8(cur);
				}
				else
				{
					u8(cur);
					return true;
				}
			}
			if (in != 0)
			{
				auto byte = (uint8_t)in;
				u8(byte);
			}
			return true;
		}

		// A signed 64-bit integer encoded in 1..9 bytes.
		bool i64_dyn(const int64_t& v)
		{
			// Split value
			uint64_t in;
			bool neg = (v < 0);
			if (neg)
			{
				in = (v * -1);
				in &= ~((uint64_t)1 << 63);
			}
			else
			{
				in = v;
			}

			// First byte
			{
				uint8_t cur = (in & 0b111111);
				cur |= (neg << 6);
				in >>= 6;
				if (in != 0)
				{
					cur |= 0x80;
					u8(cur);
				}
				else
				{
					u8(cur);
					return true;
				}
			}

			// Next 1..7 bytes
			for (uint8_t i = 0; i != 7; ++i)
			{
				uint8_t cur = (in & 0x7F);
				in >>= 7;
				if (in != 0)
				{
					cur |= 0x80;
					u8(cur);
				}
				else
				{
					u8(cur);
					return true;
				}
			}

			// Optional last byte
			if (in != 0)
			{
				auto byte = (uint8_t)in;
				u8(byte);
			}
			return true;
		}

		// An integer where every byte's most significant bit is used to indicate if another byte follows.
		template <typename Int>
		bool om(const Int& v)
		{
			Int val = v;
			while (val > 0x7F)
			{
				uint8_t byte = ((unsigned char)val | 0x80);
				u8(byte);
				val >>= 7;
			}
			uint8_t byte = (unsigned char)val;
			u8(byte);
			return true;
		}

		// Null-terminated string.
		bool str_nt(const std::string& v)
		{
			write(v.data(), v.size());
			uint8_t term = 0;
			u8(term);
			return true;
		}

		// Length-prefixed string, using u64_dyn for the length prefix.
		bool str_lp_u64_dyn(const std::string& v)
		{
			u64_dyn(v.size());
			write(v.data(), v.size());
			return true;
		}

		// Length-prefixed string, using u8 for the length prefix.
		bool str_lp_u8(const std::string& v, const uint8_t max_len = 0xFF)
		{
			size_t len = v.size();
			if (len <= max_len)
			{
				auto tl = (uint8_t)len;
				u8(tl);
				write(v.data(), v.size());
				return true;
			}
			return false;
		}

		// Length-prefixed string, using u16 for the length prefix.
		bool str_lp_u16(const std::string& v, const uint16_t max_len = 0xFFFF)
		{
			size_t len = v.size();
			if (len <= max_len)
			{
				auto tl = (uint16_t)len;
				if (IoBase::u16(tl))
				{
					write(v.data(), v.size());
					return true;
				}
			}
			return false;
		}

		// Length-prefixed string, using u24 for the length prefix.
		bool str_lp_u24(const std::string& v, const uint32_t max_len = 0xFFFFFF)
		{
			size_t len = v.size();
			if (len <= max_len)
			{
				auto tl = (uint32_t)len;
				if (IoBase::u24(tl))
				{
					write(v.data(), v.size());
					return true;
				}
			}
			return false;
		}

		// Length-prefixed string, using u32 for the length prefix.
		bool str_lp_u32(const std::string& v, const uint32_t max_len = 0xFFFFFFFF)
		{
			size_t len = v.size();
			if (len <= max_len)
			{
				auto tl = (uint32_t)len;
				if (IoBase::u32(tl))
				{
					write(v.data(), v.size());
					return true;
				}
			}
			return false;
		}

		// Length-prefixed string, using u64 for the length prefix.
		bool str_lp_u64(const std::string& v)
		{
			uint64_t len = v.size();
			if (IoBase::u64(len))
			{
				write(v.data(), v.size());
				return true;
			}
			return false;
		}

		// String with known length.
		bool str(size_t len, const std::string& v)
		{
			write(v.data(), v.size());
			return true;
		}

		// std::vector<uint8_t> with u8 size prefix.
		bool vec_u8_u8(std::vector<uint8_t>& v)
		{
			if (v.size() > 0xFF)
			{
				return false;
			}
			auto len = (uint8_t)v.size();
			if (!u8(len))
			{
				return false;
			}
			for (auto& entry : v)
			{
				if (!u8(entry))
				{
					return false;
				}
			}
			return true;
		}

		// std::vector<uint16_t> with u16 size prefix.
		bool vec_u16_u16(std::vector<uint16_t>& v)
		{
			if (v.size() > 0xFFFF)
			{
				return false;
			}
			auto len = (uint16_t)v.size();
			if (!IoBase::u16(len))
			{
				return false;
			}
			for (auto& entry : v)
			{
				if (!IoBase::u16(entry))
				{
					return false;
				}
			}
			return true;
		}

		// std::vector<uint16_t> with u16 byte length prefix.
		bool vec_u16_bl_u16(std::vector<uint16_t>& v)
		{
			size_t bl = (v.size() * sizeof(uint16_t));
			if (bl > 0xFFFF)
			{
				return false;
			}
			auto bl_u16 = (uint16_t)bl;
			if (!IoBase::u16(bl_u16))
			{
				return false;
			}
			for (auto& entry : v)
			{
				if (!IoBase::u16(entry))
				{
					return false;
				}
			}
			return true;
		}

		// vector of str_nt with u64_dyn length prefix.
		bool vec_str_nt_u64_dyn(std::vector<std::string>& v)
		{
			uint64_t len = v.size();
			if (!u64_dyn(len))
			{
				return false;
			}
			for (auto& entry : v)
			{
				if (!str_nt(entry))
				{
					return false;
				}
			}
			return true;
		}

		// vector of str_lp_u24 with u24 byte length prefix.
		bool vec_str_lp_u24_bl_u24(std::vector<std::string>& v)
		{
			size_t bl = (v.size() * 3);
			for (const auto& entry : v)
			{
				bl += entry.size();
			}
			if (bl > 0xFFFFFF)
			{
				return false;
			}
			auto bl_u32 = (uint32_t)bl;
			if (!IoBase::u24(bl_u32))
			{
				return false;
			}
			for (auto& entry : v)
			{
				if (!str_lp_u24(entry))
				{
					return false;
				}
			}
			return true;
		}
	};
}
