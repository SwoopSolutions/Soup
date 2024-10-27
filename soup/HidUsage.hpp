#pragma once

#include "base.hpp"

#include <cstdint>

NAMESPACE_SOUP
{
	union HidUsage
	{
		struct
		{
#ifdef SOUP_LITTLE_ENDIAN
			uint16_t id;
			uint16_t page;
#else
			uint16_t page;
			uint16_t id;
#endif
		};
		uint32_t value;

		HidUsage()
			: value(0)
		{
		}

		HidUsage(uint32_t value)
			: value(value)
		{
		}

		HidUsage(uint16_t page, uint16_t id)
		{
			this->page = page;
			this->id = id;
		}

		operator uint32_t& () noexcept
		{
			return value;
		}

		operator const uint32_t& () const noexcept
		{
			return value;
		}
	};
}

namespace std
{
	template <>
	struct hash<::soup::HidUsage>
	{
		size_t operator() (const ::soup::HidUsage& usage) const
		{
			hash<uint32_t> impl;
			return impl(usage.value);
		}
	};
}
