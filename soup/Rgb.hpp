#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "math.hpp"

namespace soup
{
	union Rgb
	{
		struct
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
		uint8_t arr[3];

		static const Rgb BLACK;
		static const Rgb WHITE;
		static const Rgb RED;
		static const Rgb YELLOW;
		static const Rgb GREEN;
		static const Rgb BLUE;
		static const Rgb MAGENTA;
		static const Rgb GRAY;

		constexpr Rgb() noexcept
			: r(0), g(0), b(0)
		{
		}

		template <typename TR, typename TG, typename TB>
		constexpr Rgb(TR r, TG g, TB b) noexcept
			: r((uint8_t)r), g((uint8_t)g), b((uint8_t)b)
		{
		}

		constexpr Rgb(uint32_t val) noexcept
			: r(val >> 16), g((val >> 8) & 0xFF), b(val & 0xFF)
		{
		}

		[[nodiscard]] constexpr bool operator==(const Rgb& c) const noexcept
		{
			return r == c.r && g == c.g && b == c.b;
		}

		[[nodiscard]] constexpr bool operator!=(const Rgb& c) const noexcept
		{
			return !operator==(c);
		}

		[[nodiscard]] constexpr uint32_t toInt() const noexcept
		{
			return (r << 16) | (g << 8) | b;
		}

		[[nodiscard]] std::string toHex() const;

		[[nodiscard]] static Rgb lerp(Rgb a, Rgb b, float t)
		{
			return Rgb{
				soup::lerp<uint8_t>(a.r, b.r, t),
				soup::lerp<uint8_t>(a.g, b.g, t),
				soup::lerp<uint8_t>(a.b, b.b, t),
			};
		}

		[[nodiscard]] uint8_t& operator[](unsigned int i)
		{
			return arr[i];
		}

		[[nodiscard]] const uint8_t& operator[](unsigned int i) const
		{
			return arr[i];
		}
	};
}
