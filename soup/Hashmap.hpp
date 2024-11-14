#pragma once

// VERY EXPERIMENTAL!

#include <intrin.h>

#include "alloc.hpp"
#include "bitutil.hpp"

NAMESPACE_SOUP
{
	template <typename Key, typename Value>
	class Hashmap
	{
	private:
		union Entry
		{
			struct
			{
				Key key;
				Value value;
			};
			struct
			{
				Key first;
				Value second;
			};
		};

		static constexpr size_t entries_per_area = 16;
		static constexpr size_t metadata_bytes_per_area = entries_per_area;
		static constexpr size_t data_bytes_per_area = entries_per_area * sizeof(Entry);

		void* heap = nullptr;
		size_t capacity = 0;
		size_t size = 0;

	public:
		Hashmap() = default;

		~Hashmap()
		{
			if (heap != nullptr)
			{
				soup::free(heap);
			}
		}

		[[nodiscard]] size_t getNumAreas() const noexcept
		{
			return size / entries_per_area;
		}

	private:
		[[nodiscard]] static void* heapGetMetadata(void* heap)
		{
			return heap;
		}

		[[nodiscard]] static void* heapGetData(void* heap, size_t areas)
		{
			return static_cast<uint8_t*>(heap) + (areas * metadata_bytes_per_area);
		}

		[[nodiscard]] static uint8_t* heapGetMetadataForIndex(void* heap, size_t i)
		{
			return static_cast<uint8_t*>(heap) + i;
		}

		[[nodiscard]] static Entry* heapGetDataForIndex(void* heap, size_t areas, size_t i)
		{
			return static_cast<Entry*>(heapGetData(heap, areas)) + i;
		}

	public:
		void emplace(const Key& key, const Value& value)
		{
			if (capacity == size)
			{
				constexpr size_t area_bytes = metadata_bytes_per_area + data_bytes_per_area;
				const size_t areas = getNumAreas();
				const size_t new_areas = areas + 1;
				void* const new_heap = soup::malloc(new_areas * area_bytes);
				memcpy(heapGetMetadata(new_heap), heapGetMetadata(heap), areas * metadata_bytes_per_area); // copy metadata
				memset(static_cast<uint8_t*>(new_heap) + (areas * metadata_bytes_per_area), 0, metadata_bytes_per_area); // zero out new metadata
				memcpy(heapGetData(new_heap, new_areas), heapGetData(heap, areas), capacity * sizeof(Entry)); // copy data
				if (heap != nullptr)
				{
					free(heap);
				}
				heap = new_heap;
				size += entries_per_area;
			}
			*heapGetMetadataForIndex(heap, capacity) = (uint8_t)(key & 0x7F) | 0x80;
			heapGetDataForIndex(heap, getNumAreas(), capacity)->key = key;
			heapGetDataForIndex(heap, getNumAreas(), capacity)->value = value;
			++capacity;
		}

		[[nodiscard]] Entry* find(const Key& key) const
		{
			const size_t areas = getNumAreas();
			auto match = _mm_set1_epi8((uint8_t)(key & 0x7F) | 0x80);
			uint8_t* metadata = heapGetMetadataForIndex(heap, 0);
			for (size_t i = 0; i != areas; ++i)
			{
				int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(match, _mm_loadu_si128(reinterpret_cast<__m128i*>(metadata))));
				while (mask != 0)
				{
					auto j = bitutil::getLeastSignificantSetBit(mask);
					auto entry = heapGetDataForIndex(heap, areas, (i * entries_per_area) + j);
					if (entry->key == key)
					{
						return entry;
					}
					mask &= (mask - 1); // knock out least significant set bit
				}
				metadata += 16;
			}
			return end();
		}

		[[nodiscard]] Value& at(const Key& key) const
		{
			auto entry = find(key);
			SOUP_ASSERT(entry != end());
			return entry->value;
		}

		[[nodiscard]] Entry* begin() const noexcept
		{
			return heapGetDataForIndex(heap, getNumAreas(), 0);
		}

		[[nodiscard]] Entry* end() const noexcept
		{
			return heapGetDataForIndex(heap, getNumAreas(), capacity);
		}
	};
}
