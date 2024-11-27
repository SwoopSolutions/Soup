#pragma once

#include "base.hpp"

#include <cstdint>

NAMESPACE_SOUP
{
	struct HidUsage
	{
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
			: value((static_cast<uint32_t>(page) << 16) | id)
		{
		}

		[[nodiscard]] uint16_t getPage() const noexcept
		{
			return value >> 16;
		}

		[[nodiscard]] uint16_t getId() const noexcept
		{
			return static_cast<uint16_t>(value);
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
