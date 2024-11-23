#pragma once

#include "Packet.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(MysqlHandshakeResponse)
	{
		u32 client_capabilities;
		u32 max_packet;
		u8 charset;
		std::string username;
		std::string password;
		std::string client_auth_plugin;

		SOUP_PACKET_IO(s)
		{
			return s.u32le(client_capabilities)
				&& s.u32le(max_packet)
				&& s.u8(charset)
				&& s.skip(23)
				&& s.str_nt(username)
				&& s.str_lp_mysql(password)
				&& s.str_nt(client_auth_plugin)
				;
		}
	};
}
