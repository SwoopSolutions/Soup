#pragma once

#include "Packet.hpp"

#include <utility>
#include <vector>

#include "MacAddr.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(DhcpMessage)
	{
		u8 type;
		u8 hw_type = 1; // Ethernet
		u8 hw_addr_len = 6;
		u8 hops = 0;
		u32 transaction_id;
		u16 secs_elapsed = 0;
		u16 flags = 0;
		u32 client_addr = 0;
		u32 your_addr = 0;
		u32 server_addr = 0;
		u32 gateway_addr = 0;
		MacAddr mac_addr = MacAddr::SOUP_FIRST;
		std::vector<std::pair<u8, std::string>> options{};

		void addOption(u8 type, std::string&& data)
		{
			options.emplace_back(type, std::move(data));
		}

		[[nodiscard]] std::string* findOption(u8 type)
		{
			for (auto& option : options)
			{
				if (option.first == type)
				{
					return &option.second;
				}
			}
			return nullptr;
		}

		SOUP_PACKET_IO(s)
		{
			u32 magic_cookie = 0x63825363;
			if (!s.u8(type)
				|| !s.u8(hw_type)
				|| !s.u8(hw_addr_len)
				|| !s.u8(hops)
				|| !s.u32be(transaction_id)
				|| !s.u16be(secs_elapsed)
				|| !s.u16be(flags)
				|| !s.u32be(client_addr)
				|| !s.u32be(your_addr)
				|| !s.u32be(server_addr)
				|| !s.u32be(gateway_addr)
				|| !mac_addr.io(s)
				|| !s.skip(10) // MAC padding
				|| !s.skip(192) // BOOTP legacy
				|| !s.u32be(magic_cookie)
				)
			{
				return false;
			}

			SOUP_IF_ISREAD
			{
				if (magic_cookie != 0x63825363)
				{
					return false;
				}
				options.clear();
				for (u8 type; s.u8(type), type != 0xFF; )
				{
					std::string data;
					if (!s.template str_lp<u8_t>(data))
					{
						return false;
					}
					options.emplace_back(type, std::move(data));
				}
			}
			SOUP_ELSEIF_ISWRITE
			{
				for (auto& option : options)
				{
					if (!s.u8(option.first)
						|| !s.template str_lp<u8_t>(option.second)
						)
					{
						return false;
					}
				}
				u8 endmark = 0xFF;
				if (!s.u8(endmark))
				{
					return false;
				}
			}
			return true;
		}
	};
}
