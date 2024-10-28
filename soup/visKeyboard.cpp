#include "visKeyboard.hpp"

#include "RasterFont.hpp"
#include "RenderTarget.hpp"
#include "Rgb.hpp"

NAMESPACE_SOUP
{
	void visKeyboard::draw(RenderTarget& rt, unsigned int x, unsigned int y, uint8_t scale) const
	{
		// Function row
		drawKey(rt, scale, nullptr, values[KEY_ESCAPE], x + 0 * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F1], x + (20 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F2], x + (30 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F3], x + (40 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F4], x + (50 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F5], x + (65 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F6], x + (75 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F7], x + (85 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F8], x + (95 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F9], x + (110 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F10], x + (120 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F11], x + (130 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_F12], x + (140 - 1) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_PRINT_SCREEN], x + 154 * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_PAUSE], x + 164 * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_SCROLL_LOCK], x + 174 * scale, y + 0 * scale);

		y += 15 * scale;

		// Main keyboard chunk
		drawKey(rt, scale, nullptr, values[KEY_BACKQUOTE], x + 0 * scale, y + 0 * scale);
		drawKey(rt, scale, "1", values[KEY_1], x + (0 + 10) * scale, y + 0 * scale);
		drawKey(rt, scale, "2", values[KEY_2], x + (0 + 20) * scale, y + 0 * scale);
		drawKey(rt, scale, "3", values[KEY_3], x + (0 + 30) * scale, y + 0 * scale);
		drawKey(rt, scale, "4", values[KEY_4], x + (0 + 40) * scale, y + 0 * scale);
		drawKey(rt, scale, "5", values[KEY_5], x + (0 + 50) * scale, y + 0 * scale);
		drawKey(rt, scale, "6", values[KEY_6], x + (0 + 60) * scale, y + 0 * scale);
		drawKey(rt, scale, "7", values[KEY_7], x + (0 + 70) * scale, y + 0 * scale);
		drawKey(rt, scale, "8", values[KEY_8], x + (0 + 80) * scale, y + 0 * scale);
		drawKey(rt, scale, "9", values[KEY_9], x + (0 + 90) * scale, y + 0 * scale);
		drawKey(rt, scale, "0", values[KEY_0], x + (0 + 100) * scale, y + 0 * scale);
		drawKey(rt, scale, "-", values[KEY_MINUS], x + (0 + 110) * scale, y + 0 * scale);
		drawKey(rt, scale, "=", values[KEY_EQUALS], x + (0 + 120) * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_BACKSPACE], x + (0 + 130) * scale, y + 0 * scale, 19);
		drawKey(rt, scale, nullptr, values[KEY_TAB], x + 0 * scale, y + 10 * scale, 15);
		drawKey(rt, scale, "Q", values[KEY_Q], x + (5 + 10) * scale, y + 10 * scale);
		drawKey(rt, scale, "W", values[KEY_W], x + (5 + 20) * scale, y + 10 * scale);
		drawKey(rt, scale, "E", values[KEY_E], x + (5 + 30) * scale, y + 10 * scale);
		drawKey(rt, scale, "R", values[KEY_R], x + (5 + 40) * scale, y + 10 * scale);
		drawKey(rt, scale, "T", values[KEY_T], x + (5 + 50) * scale, y + 10 * scale);
		drawKey(rt, scale, "Y", values[KEY_Y], x + (5 + 60) * scale, y + 10 * scale);
		drawKey(rt, scale, "U", values[KEY_U], x + (5 + 70) * scale, y + 10 * scale);
		drawKey(rt, scale, "I", values[KEY_I], x + (5 + 80) * scale, y + 10 * scale);
		drawKey(rt, scale, "O", values[KEY_O], x + (5 + 90) * scale, y + 10 * scale);
		drawKey(rt, scale, "P", values[KEY_P], x + (5 + 100) * scale, y + 10 * scale);
		drawKey(rt, scale, "[", values[KEY_BRACKET_LEFT], x + (5 + 110) * scale, y + 10 * scale);
		drawKey(rt, scale, "]", values[KEY_BRACKET_RIGHT], x + (5 + 120) * scale, y + 10 * scale);
		drawKey(rt, scale, nullptr, values[KEY_ENTER], x + (5 + 130) * scale, y + 10 * scale, 14);
		drawKey(rt, scale, nullptr, values[KEY_CAPS_LOCK], x + 0 * scale, y + 20 * scale, 18);
		drawKey(rt, scale, "A", values[KEY_A], x + (8 + 10) * scale, y + 20 * scale);
		drawKey(rt, scale, "S", values[KEY_S], x + (8 + 20) * scale, y + 20 * scale);
		drawKey(rt, scale, "D", values[KEY_D], x + (8 + 30) * scale, y + 20 * scale);
		drawKey(rt, scale, "F", values[KEY_F], x + (8 + 40) * scale, y + 20 * scale);
		drawKey(rt, scale, "G", values[KEY_G], x + (8 + 50) * scale, y + 20 * scale);
		drawKey(rt, scale, "H", values[KEY_H], x + (8 + 60) * scale, y + 20 * scale);
		drawKey(rt, scale, "J", values[KEY_J], x + (8 + 70) * scale, y + 20 * scale);
		drawKey(rt, scale, "K", values[KEY_K], x + (8 + 80) * scale, y + 20 * scale);
		drawKey(rt, scale, "L", values[KEY_L], x + (8 + 90) * scale, y + 20 * scale);
		drawKey(rt, scale, ";", values[KEY_SEMICOLON], x + (8 + 100) * scale, y + 20 * scale);
		drawKey(rt, scale, "'", values[KEY_QUOTE], x + (8 + 110) * scale, y + 20 * scale);
		drawKey(rt, scale, "\\", values[KEY_BACKSLASH], x + (8 + 120) * scale, y + 20 * scale);
		drawKey(rt, scale, nullptr, values[KEY_ENTER], x + (8 + 130) * scale, y + 20 * scale, 11);
		drawKey(rt, scale, nullptr, values[KEY_LSHIFT], x + 0 * scale, y + 30 * scale, 12);
		drawKey(rt, scale, nullptr, values[KEY_INTL_BACKSLASH], x + 12 * scale, y + 30 * scale);
		drawKey(rt, scale, "Z", values[KEY_Z], x + (12 + 10) * scale, y + 30 * scale);
		drawKey(rt, scale, "X", values[KEY_X], x + (12 + 20) * scale, y + 30 * scale);
		drawKey(rt, scale, "C", values[KEY_C], x + (12 + 30) * scale, y + 30 * scale);
		drawKey(rt, scale, "V", values[KEY_V], x + (12 + 40) * scale, y + 30 * scale);
		drawKey(rt, scale, "B", values[KEY_B], x + (12 + 50) * scale, y + 30 * scale);
		drawKey(rt, scale, "N", values[KEY_N], x + (12 + 60) * scale, y + 30 * scale);
		drawKey(rt, scale, "M", values[KEY_M], x + (12 + 70) * scale, y + 30 * scale);
		drawKey(rt, scale, ",", values[KEY_COMMA], x + (12 + 80) * scale, y + 30 * scale);
		drawKey(rt, scale, ".", values[KEY_PERIOD], x + (12 + 90) * scale, y + 30 * scale);
		drawKey(rt, scale, "/", values[KEY_SLASH], x + (12 + 100) * scale, y + 30 * scale);
		drawKey(rt, scale, nullptr, values[KEY_RSHIFT], x + (12 + 110) * scale, y + 30 * scale, 27);
		drawKey(rt, scale, nullptr, values[KEY_LCTRL], x + 0 * scale, y + 40 * scale,  12);
		drawKey(rt, scale, nullptr, values[KEY_LMETA], x + 12 * scale, y + 40 * scale, 12);
		drawKey(rt, scale, nullptr, values[KEY_LALT], x + 24 * scale, y + 40 * scale, 12);
		drawKey(rt, scale, nullptr, values[KEY_SPACE], x + 36 * scale, y + 40 * scale, 65);
		drawKey(rt, scale, nullptr, values[KEY_RALT], x + 101 * scale, y + 40 * scale, 12);
		if (!has_ctx_key)
		{
			drawKey(rt, scale, nullptr, values[KEY_RMETA], x + 113 * scale, y + 40 * scale, 12);
			drawKey(rt, scale, nullptr, values[KEY_FN], x + 125 * scale, y + 40 * scale, 12);
		}
		else
		{
			drawKey(rt, scale, nullptr, values[KEY_FN], x + 113 * scale, y + 40 * scale, 12);
			drawKey(rt, scale, nullptr, values[KEY_CTX], x + 125 * scale, y + 40 * scale, 12);
		}
		drawKey(rt, scale, nullptr, values[KEY_RCTRL], x + 137 * scale, y + 40 * scale, 12);

		// Navigation keys
		drawKey(rt, scale, nullptr, values[KEY_INSERT], x + 154 * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_HOME], x + 164 * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_PAGE_UP], x + 174 * scale, y + 0 * scale);
		drawKey(rt, scale, nullptr, values[KEY_DEL], x + 154 * scale, y + 10 * scale);
		drawKey(rt, scale, nullptr, values[KEY_END], x + 164 * scale, y + 10 * scale);
		drawKey(rt, scale, nullptr, values[KEY_PAGE_DOWN], x + 174 * scale, y + 10 * scale);
		drawKey(rt, scale, nullptr, values[KEY_ARROW_UP], x + 164 * scale, y + 30 * scale);
		drawKey(rt, scale, nullptr, values[KEY_ARROW_LEFT], x + 154 * scale, y + 40 * scale);
		drawKey(rt, scale, nullptr, values[KEY_ARROW_DOWN], x + 164 * scale, y + 40 * scale);
		drawKey(rt, scale, nullptr, values[KEY_ARROW_RIGHT], x + 174 * scale, y + 40 * scale);

		x += 189 * scale;

		drawKey(rt, scale, nullptr, values[KEY_NUM_LOCK], x + 0 * scale, y + 0 * scale);
		drawKey(rt, scale, "/", values[KEY_NUMPAD_DIVIDE], x + 10 * scale, y + 0 * scale);
		drawKey(rt, scale, "*", values[KEY_NUMPAD_MULTIPLY], x + 20 * scale, y + 0 * scale);
		drawKey(rt, scale, "-", values[KEY_NUMPAD_SUBTRACT], x + 30 * scale, y + 0 * scale);
		drawKey(rt, scale, "7", values[KEY_NUMPAD7], x + 0 * scale, y + 10 * scale);
		drawKey(rt, scale, "8", values[KEY_NUMPAD8], x + 10 * scale, y + 10 * scale);
		drawKey(rt, scale, "9", values[KEY_NUMPAD9], x + 20 * scale, y + 10 * scale);
		drawKey(rt, scale, "+", values[KEY_NUMPAD_ADD], x + 30 * scale, y + 10 * scale, 10, 20);
		drawKey(rt, scale, "4", values[KEY_NUMPAD4], x + 0 * scale, y + 20 * scale);
		drawKey(rt, scale, "5", values[KEY_NUMPAD5], x + 10 * scale, y + 20 * scale);
		drawKey(rt, scale, "6", values[KEY_NUMPAD6], x + 20 * scale, y + 20 * scale);
		drawKey(rt, scale, "1", values[KEY_NUMPAD1], x + 0 * scale, y + 30 * scale);
		drawKey(rt, scale, "2", values[KEY_NUMPAD2], x + 10 * scale, y + 30 * scale);
		drawKey(rt, scale, "3", values[KEY_NUMPAD3], x + 20 * scale, y + 30 * scale);
		drawKey(rt, scale, nullptr, values[KEY_NUMPAD_ENTER], x + 30 * scale, y + 30 * scale, 10, 20);
		drawKey(rt, scale, "0", values[KEY_NUMPAD0], x + 0 * scale, y + 40 * scale, 20);
		drawKey(rt, scale, ".", values[KEY_NUMPAD_DECIMAL], x + 20 * scale, y + 40 * scale);
	}

	void visKeyboard::drawKey(RenderTarget& rt, uint8_t scale, const char* label, uint8_t value, unsigned int x, unsigned int y, unsigned int width, unsigned int height) const
	{
		const RasterFont& font = RasterFont::simple5();
		rt.drawRect(x, y, width * scale, height * scale, Rgb::lerp(Rgb::BLACK, Rgb::MAGENTA, static_cast<float>(value) / 255.0f));
		if (label)
		{
			rt.drawCentredText(x + (width * scale / 2), y + ((height - font.baseline_glyph_height) * scale / 2), label, font, Rgb::WHITE, scale);
		}
	}
}
