#pragma once

#include "base.hpp"
#if SOUP_WINDOWS && SOUP_X86

#include "Packet.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(Hwid)
	{
		u32 cpu_hash;
		u24 gpu_hash;
		u8 ram_mag;

		[[nodiscard]] static Hwid get();

		SOUP_PACKET_IO(s)
		{
			return s.u32be(cpu_hash)
				&& s.u24be(gpu_hash)
				&& s.u8(ram_mag)
				;
		}
	};
}

#endif
