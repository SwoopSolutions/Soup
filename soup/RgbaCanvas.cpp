#include "RgbaCanvas.hpp"

#include "base.hpp"

#include "Canvas.hpp"
#include "Writer.hpp"

NAMESPACE_SOUP
{
	void RgbaCanvas::fill(const Rgba colour)
	{
		for (auto& p : pixels)
		{
			p = colour;
		}
	}

	void RgbaCanvas::set(unsigned int x, unsigned int y, Rgba colour) noexcept
	{
		SOUP_IF_LIKELY (x < width && y < height)
		{
			pixels.at(x + (y * width)) = colour;
		}
	}

	Rgba RgbaCanvas::get(unsigned int x, unsigned int y) const
	{
		return pixels.at(x + (y * width));
	}

	const Rgba& RgbaCanvas::ref(unsigned int x, unsigned int y) const
	{
		return pixels.at(x + (y * width));
	}

	void RgbaCanvas::subtractGreen()
	{
		for (auto& p : pixels)
		{
			if (p.g != 0)
			{
				const int rbavg = ((p.r + p.b) / 2);
				if (p.g > rbavg)
				{
					const auto extra_green = (p.g - rbavg);
					p.g -= extra_green;
					p.a = (0xFF - extra_green);
				}
			}
		}
	}

	RgbaCanvas RgbaCanvas::fromCanvas(const Canvas& solid, uint8_t alpha)
	{
		RgbaCanvas c;
		c.width = solid.width;
		c.height = solid.height;
		c.pixels.resize(solid.pixels.size());
		for (size_t i = 0; i != solid.pixels.size(); ++i)
		{
			c.pixels.at(i).rgb = solid.pixels.at(i);
			c.pixels.at(i).a = alpha;
		}
		return c;
	}

	bool RgbaCanvas::toBmp(Writer& w) const
	{
		// Based on Canvas::toBmp + https://github.com/openfl/lime/blob/4adb7d638d15612e2542ae3a0ef581a786d21183/src/lime/_internal/format/BMP.hx
		uint8_t b;
		uint16_t s;
		uint32_t i;
		SOUP_IF_UNLIKELY (!(s = 0x4D42, w.u16le(s))
			|| !(i = static_cast<uint32_t>(108 + (4 * pixels.size())), w.u32le(i))
			|| !w.skip(4)
			|| !(i = (14 + 108), w.u32le(i))
			|| !(i = 108, w.u32le(i))
			|| !(i = width, w.u32le(i))
			|| !(i = height * -1, w.u32le(i))
			|| !(s = 1, w.u16le(s))
			|| !(s = 32, w.u16le(s))
			|| !(i = 3, w.u32le(i))
			|| !(i = static_cast<uint32_t>(4 * pixels.size()), w.u32le(i))
			|| !w.skip(4 + 4 + 4 + 4)
			|| !(i = 0x00FF0000, w.u32le(i))
			|| !(i = 0x0000FF00, w.u32le(i))
			|| !(i = 0x000000FF, w.u32le(i))
			|| !(i = 0xFF000000, w.u32le(i))
			|| !(b = 0x20, w.u8(b))
			|| !(b = 0x6E, w.u8(b))
			|| !(b = 0x69, w.u8(b))
			|| !(b = 0x57, w.u8(b))
			|| !w.skip(48)
			)
		{
			return false;
		}
		for (const auto& p : pixels)
		{
			SOUP_IF_UNLIKELY (!w.u8(const_cast<uint8_t&>(p.r))
				|| !w.u8(const_cast<uint8_t&>(p.g))
				|| !w.u8(const_cast<uint8_t&>(p.b))
				|| !w.u8(const_cast<uint8_t&>(p.a))
				)
			{
				break;
			}
		}
		return true;
	}
}
