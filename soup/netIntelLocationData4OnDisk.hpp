#pragma once

#include "Packet.hpp"

#include "spaceship.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(netIntelLocationData4OnDisk)
	{
		uint32_t lower;
		uint32_t upper;

		std::string country_code;
		uint32_t state_offset;
		uint32_t city_offset;

		SOUP_PACKET_IO(s)
		{
			return s.u32le(lower)
				&& s.u32le(upper)
				&& s.str(2, country_code)
				&& s.u32le(state_offset)
				&& s.u32le(city_offset)
				;
		}

		[[nodiscard]] static soup::strong_ordering cmp(Reader& r, const uint32_t& hint)
		{
			netIntelLocationData4OnDisk data;
			r.u32le(data.lower);
			r.u32le(data.upper);
			if (data.lower <= hint && hint <= data.upper)
			{
				return soup::strong_ordering::equal;
			}
			return data.lower < hint ? soup::strong_ordering::greater : soup::strong_ordering::less;
		}
	};
}
