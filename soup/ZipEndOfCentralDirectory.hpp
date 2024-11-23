#pragma once

#include "Packet.hpp"

#include <string>

NAMESPACE_SOUP
{
	SOUP_PACKET(ZipEndOfCentralDirectory)
	{
		u16 this_disk_number;
		u16 central_directory_disk;
		u16 central_directories_on_this_disk;
		u16 central_directories_in_total;
		u32 central_directory_size;
		u32 central_directory_offset; // relative to start of archive
		std::string comment;

		SOUP_PACKET_IO(s)
		{
			if (!s.u16le(this_disk_number)
				|| !s.u16le(central_directory_disk)
				|| !s.u16le(central_directories_on_this_disk)
				|| !s.u16le(central_directories_in_total)
				|| !s.u32le(central_directory_size)
				|| !s.u32le(central_directory_offset)
				|| !s.template str_lp<u16le_t>(comment)
				)
			{
				return false;
			}
			return true;
		}
	};
}
