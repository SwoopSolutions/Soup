#pragma once

#include "ZipFileCommon.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(ZipCentralDirectoryFile)
	{
		u16 version_made_by;
		ZipFileCommon common;
		std::string name;
		std::string extra;
		std::string comment;
		u16 disk;
		u16 internal_attributes;
		u32 external_attributes;
		u32 disk_offset;

		SOUP_PACKET_IO(s)
		{
			if (!s.u16le(version_made_by)
				|| !common.io(s))
			{
				return false;
			}
			auto name_length = (u16)name.size();
			auto extra_length = (u16)extra.size();
			auto comment_length = (u16)comment.size();
			return s.u16le(name_length)
				&& s.u16le(extra_length)
				&& s.u16le(comment_length)
				&& s.u16le(disk)
				&& s.u16le(internal_attributes)
				&& s.u32le(external_attributes)
				&& s.u32le(disk_offset)
				&& s.str(name_length, name)
				&& s.str(extra_length, extra)
				&& s.str(comment_length, comment)
				;
		}
	};
}
