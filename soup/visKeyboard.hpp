#pragma once

#include <cstring> // memset

#include "fwd.hpp"

#include "Key.hpp"

NAMESPACE_SOUP
{
	class visKeyboard
	{
	public:
		uint8_t values[NUM_KEYS];
		bool has_ctx_key = false;

		visKeyboard() noexcept
		{
			clear();
		}

		void clear() noexcept
		{
			memset(values, 0, sizeof(values));
		}

		static constexpr auto WIDTH = 229;
		static constexpr auto HEIGHT = 65;

		void draw(RenderTarget& rt, unsigned int x, unsigned int y, uint8_t scale = 1) const;
	protected:
		void drawKey(RenderTarget& rt, uint8_t scale, const char* label, uint8_t value, unsigned int x, unsigned int y, unsigned int width = 10, unsigned int height = 10) const;
	};
}
