#pragma once

#include "Packet.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(ZipFileCommon)
	{
		u16 min_extract_version;
		u16 bitflag;
		u16 compression_method;
		u16 last_mod_time;
		u16 last_mod_date;
		u32 uncompressed_data_crc32;
		u32 compressed_size;
		u32 uncompressed_size;

		SOUP_PACKET_IO(s)
		{
			return s.u16le(min_extract_version)
				&& s.u16le(bitflag)
				&& s.u16le(compression_method)
				&& s.u16le(last_mod_time)
				&& s.u16le(last_mod_date)
				&& s.u32le(uncompressed_data_crc32)
				&& s.u32le(compressed_size)
				&& s.u32le(uncompressed_size)
				;
		}
	};
}
