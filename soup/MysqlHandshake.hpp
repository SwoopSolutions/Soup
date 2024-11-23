#pragma once

#include "Packet.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(MysqlHandshake)
	{
		u8 protocol_version;
		std::string human_readable_version;
		u32 thread_id;
		std::string salt_pt_1;
		u16 capabilities_lo;
		u8 language;
		u16 status;
		u16 capabilities_hi;
		u8 auth_plugin_data_len;
		u32 mariadb_capabilities;
		std::string salt_pt_2;
		std::string auth_plugin_name;

		SOUP_PACKET_IO(s)
		{
			return s.u8(protocol_version)
				&& s.str_nt(human_readable_version)
				&& s.u32le(thread_id)
				&& s.str(8, salt_pt_1)
				&& s.skip(1)
				&& s.u16le(capabilities_lo)
				&& s.u8(language)
				&& s.u16le(status)
				&& s.u16le(capabilities_hi)
				&& s.u8(auth_plugin_data_len)
				&& s.skip(6)
				&& s.u32le(mariadb_capabilities)
				&& s.str(12, salt_pt_2)
				&& s.skip(1)
				&& s.str_nt(auth_plugin_name)
				;
		}
	};
}
