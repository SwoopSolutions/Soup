#pragma once

#include <cstdint>

#include "base.hpp"

NAMESPACE_SOUP
{
	enum Key : uint8_t
	{
		KEY_NONE = 0xff,

		KEY_ESCAPE = 0,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,

		KEY_BACKQUOTE,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_0,
		KEY_MINUS,
		KEY_EQUALS,
		KEY_BACKSPACE,
		KEY_TAB,
		KEY_Q,
		KEY_W,
		KEY_E,
		KEY_R,
		KEY_T,
		KEY_Y,
		KEY_U,
		KEY_I,
		KEY_O,
		KEY_P,
		KEY_BRACKET_LEFT,
		KEY_BRACKET_RIGHT,
		KEY_ENTER,
		KEY_CAPS_LOCK,
		KEY_A,
		KEY_S,
		KEY_D,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_SEMICOLON,
		KEY_QUOTE,
		KEY_BACKSLASH,
		KEY_LSHIFT,
		KEY_INTL_BACKSLASH,
		KEY_Z,
		KEY_X,
		KEY_C,
		KEY_V,
		KEY_B,
		KEY_N,
		KEY_M,
		KEY_COMMA,
		KEY_PERIOD,
		KEY_SLASH,
		KEY_RSHIFT,
		KEY_LCTRL,
		KEY_LMETA,
		KEY_LALT,
		KEY_IME_OFF,
		KEY_SPACE,
		KEY_IME_ON,
		KEY_RALT,
		KEY_RMETA, // Physical arrangement: [RMETA] [FN] [RCTRL]
		KEY_FN,
		KEY_CTX, // Physical arrangement - Razer: [FN] [CTX] [RCTRL], Madlions: [CTX] [RCTRL] [FN]
		KEY_RCTRL,

		KEY_PRINT_SCREEN,
		KEY_PAUSE,
		KEY_SCROLL_LOCK,

		KEY_INSERT,
		KEY_HOME,
		KEY_PAGE_UP,
		KEY_DEL,
		KEY_END,
		KEY_PAGE_DOWN,

		KEY_ARROW_UP,
		KEY_ARROW_LEFT,
		KEY_ARROW_DOWN,
		KEY_ARROW_RIGHT,

		KEY_NUM_LOCK,
		KEY_NUMPAD_DIVIDE,
		KEY_NUMPAD_MULTIPLY,
		KEY_NUMPAD_SUBTRACT,
		KEY_NUMPAD7,
		KEY_NUMPAD8,
		KEY_NUMPAD9,
		KEY_NUMPAD_ADD,
		KEY_NUMPAD4,
		KEY_NUMPAD5,
		KEY_NUMPAD6,
		KEY_NUMPAD1,
		KEY_NUMPAD2,
		KEY_NUMPAD3,
		KEY_NUMPAD_ENTER,
		KEY_NUMPAD0,
		KEY_NUMPAD_DECIMAL,

		KEY_STOP_MEDIA,
		KEY_PREV_TRACK,
		KEY_PLAY_PAUSE,
		KEY_NEXT_TRACK,

		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_F16,
		KEY_F17,
		KEY_F18,
		KEY_F19,
		KEY_F20,
		KEY_F21,
		KEY_F22,

		KEY_OEM_1,
		KEY_OEM_2,
		KEY_OEM_3,
		KEY_OEM_4,
		KEY_OEM_5,
		KEY_OEM_6,
		KEY_OEM_7,
		KEY_OEM_8,
		KEY_OEM_9,
		KEY_OEM_10,

		NUM_KEYS
	};

#if SOUP_WINDOWS
	[[nodiscard]] int soup_key_to_virtual_key(uint8_t key) noexcept; // E.g. KEY_Y -> 'Y'
	[[nodiscard]] int soup_key_to_translated_virtual_key(uint8_t key) noexcept; // E.g. KEY_Y -> 'Y' or 'Z' depending on user's layout

	[[nodiscard]] Key virtual_key_to_soup_key(int vk) noexcept;
#endif

	[[nodiscard]] uint16_t soup_key_to_ps2_scancode(uint8_t key) noexcept;
	[[nodiscard]] Key ps2_scancode_to_soup_key(uint16_t scancode) noexcept;
}
