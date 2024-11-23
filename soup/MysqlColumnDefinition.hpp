#pragma once

#include "Packet.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(MysqlColumnDefinition)
	{
		std::string catalog; // currently always "def"
		std::string schema;
		std::string table;
		std::string org_table;
		std::string name; // the actually relevant thing
		std::string org_name;
		u16 character_set;
		u32 column_length;
		u8 type; // MysqlFieldTypes
		u16 flags;
		u8 decimals;

		SOUP_PACKET_IO(s)
		{
			if (!s.str_lp_mysql(catalog)
				|| !s.str_lp_mysql(schema)
				|| !s.str_lp_mysql(table)
				|| !s.str_lp_mysql(org_table)
				|| !s.str_lp_mysql(name)
				|| !s.str_lp_mysql(org_name))
			{
				return false;
			}
			uint8_t buf = 0x0c;
			return s.u8(buf)
				&& s.u16le(character_set)
				&& s.u32le(column_length)
				&& s.u8(type)
				&& s.u16le(flags)
				&& s.u8(decimals)
				;
		}
	};
}
