#pragma once

#include <unordered_map>

#include "base.hpp"

NAMESPACE_SOUP
{
	template <typename K, typename V>
	struct UnorderedMap : public std::unordered_map<K, V>
	{
		using std::unordered_map<K, V>::unordered_map;

		[[nodiscard]] V get_or_default(K key, V fallback) const noexcept
		{
			if (auto e = this->find(key); e != this->end())
			{
				return e->second;
			}
			return fallback;
		}
	};
}
