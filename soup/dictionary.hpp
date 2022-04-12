#pragma once

#include "fwd.hpp"

#include "dictionary_word.hpp"
#include <map>

namespace soup
{
	SOUP_PACKET(dictionary)
	{
		std::map<std::string, dictionary_word> words{};

		void addWords(const json_object& obj); // Made for data from https://stackoverflow.com/a/54982015

		[[nodiscard]] const dictionary_word* find(std::string word) const;
		[[nodiscard]] const dictionary_word& at(std::string word) const;

		SOUP_PACKET_IO(s)
		{
			u8 format_version = 0;
			if (!s.u8(format_version)
				|| format_version != 0
				)
			{
				return false;
			}
			if constexpr (s.isRead())
			{
				while (s.hasMore())
				{
					std::string word{};
					dictionary_word dw{};
					if (!s.str_nt(word)
						|| !dw.io(s)
						)
					{
						return false;
					}
					words.emplace(std::move(word), std::move(dw));
				}
			}
			else if constexpr (s.isWrite())
			{
				for (auto& e : words)
				{
					if (!s.str_nt(e.first)
						|| !e.second.io(s))
					{
						return false;
					}
				}
			}
			return true;
		}
	};
}
