#pragma once

#include "Packet.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(MysqlFrame)
	{
		u24 length;
		u8 seq_id;

		SOUP_PACKET_IO(s)
		{
			return s.u24le(length)
				&& s.u8(seq_id)
				;
		}
	};
}
