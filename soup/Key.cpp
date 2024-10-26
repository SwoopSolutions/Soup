#include "Key.hpp"

#include "Ps2Scancode.hpp"

#if SOUP_WINDOWS
#include <windows.h>
#endif

NAMESPACE_SOUP
{
#if SOUP_WINDOWS
	int soup_key_to_virtual_key(uint8_t key) noexcept
	{
		switch (key)
		{
		case KEY_ESCAPE: return VK_ESCAPE;
		case KEY_F1: return VK_F1;
		case KEY_F2: return VK_F2;
		case KEY_F3: return VK_F3;
		case KEY_F4: return VK_F4;
		case KEY_F5: return VK_F5;
		case KEY_F6: return VK_F6;
		case KEY_F7: return VK_F7;
		case KEY_F8: return VK_F8;
		case KEY_F9: return VK_F9;
		case KEY_F10: return VK_F10;
		case KEY_F11: return VK_F11;
		case KEY_F12: return VK_F12;

		case KEY_BACKQUOTE: return VK_OEM_3;
		case KEY_1: return '1';
		case KEY_2: return '2';
		case KEY_3: return '3';
		case KEY_4: return '4';
		case KEY_5: return '5';
		case KEY_6: return '6';
		case KEY_7: return '7';
		case KEY_8: return '8';
		case KEY_9: return '9';
		case KEY_0: return '0';
		case KEY_MINUS: return VK_OEM_MINUS;
		case KEY_EQUALS: return VK_OEM_PLUS;
		case KEY_BACKSPACE: return VK_BACK;
		case KEY_TAB: return VK_TAB;
		case KEY_Q: return 'Q';
		case KEY_W: return 'W';
		case KEY_E: return 'E';
		case KEY_R: return 'R';
		case KEY_T: return 'T';
		case KEY_Y: return 'Y';
		case KEY_U: return 'U';
		case KEY_I: return 'I';
		case KEY_O: return 'O';
		case KEY_P: return 'P';
		case KEY_BRACKET_LEFT: return VK_OEM_4;
		case KEY_BRACKET_RIGHT: return VK_OEM_6;
		case KEY_ENTER: return VK_RETURN;
		case KEY_CAPS_LOCK: return VK_CAPITAL;
		case KEY_A: return 'A';
		case KEY_S: return 'S';
		case KEY_D: return 'D';
		case KEY_F: return 'F';
		case KEY_G: return 'G';
		case KEY_H: return 'H';
		case KEY_J: return 'J';
		case KEY_K: return 'K';
		case KEY_L: return 'L';
		case KEY_SEMICOLON: return VK_OEM_1;
		case KEY_QUOTE: return VK_OEM_7;
		case KEY_BACKSLASH: return VK_OEM_5;
		case KEY_LSHIFT: return VK_LSHIFT;
		case KEY_INTL_BACKSLASH: return VK_OEM_102;
		case KEY_Z: return 'Z';
		case KEY_X: return 'X';
		case KEY_C: return 'C';
		case KEY_V: return 'V';
		case KEY_B: return 'B';
		case KEY_N: return 'N';
		case KEY_M: return 'M';
		case KEY_COMMA: return VK_OEM_COMMA;
		case KEY_PERIOD: return VK_OEM_PERIOD;
		case KEY_SLASH: return VK_OEM_2;
		case KEY_RSHIFT: return VK_RSHIFT;
		case KEY_LCTRL: return VK_LCONTROL;
		case KEY_LMETA: return VK_LWIN;
		case KEY_LALT: return VK_LMENU;
		case KEY_SPACE: return VK_SPACE;
		case KEY_RALT: return VK_RMENU;
		case KEY_RMETA: return VK_RWIN;
		//case KEY_FN: return VK_FN;
		case KEY_CTX: return VK_APPS;
		case KEY_RCTRL: return VK_RCONTROL;

		case KEY_PRINT_SCREEN: return VK_SNAPSHOT;
		case KEY_PAUSE: return VK_PAUSE;
		case KEY_SCROLL_LOCK: return VK_SCROLL;

		case KEY_INSERT: return VK_INSERT;
		case KEY_HOME: return VK_HOME;
		case KEY_PAGE_UP: return VK_PRIOR;
		case KEY_DEL: return VK_DELETE;
		case KEY_END: return VK_END;
		case KEY_PAGE_DOWN: return VK_NEXT;

		case KEY_ARROW_UP: return VK_UP;
		case KEY_ARROW_LEFT: return VK_LEFT;
		case KEY_ARROW_DOWN: return VK_DOWN;
		case KEY_ARROW_RIGHT: return VK_RIGHT;

		case KEY_NUM_LOCK: return VK_NUMLOCK;
		case KEY_NUMPAD_DIVIDE: return VK_DIVIDE;
		case KEY_NUMPAD_MULTIPLY: return VK_MULTIPLY;
		case KEY_NUMPAD_SUBTRACT: return VK_SUBTRACT;
		case KEY_NUMPAD7: return VK_NUMPAD7;
		case KEY_NUMPAD8: return VK_NUMPAD8;
		case KEY_NUMPAD9: return VK_NUMPAD9;
		case KEY_NUMPAD_ADD: return VK_ADD;
		case KEY_NUMPAD4: return VK_NUMPAD4;
		case KEY_NUMPAD5: return VK_NUMPAD5;
		case KEY_NUMPAD6: return VK_NUMPAD6;
		case KEY_NUMPAD1: return VK_NUMPAD1;
		case KEY_NUMPAD2: return VK_NUMPAD2;
		case KEY_NUMPAD3: return VK_NUMPAD3;
		case KEY_NUMPAD_ENTER: return VK_RETURN; // There were plans to allocate VK_NUMPAD_RETURN as 0x0E, but for now this is not the case.
		case KEY_NUMPAD0: return VK_NUMPAD0;
		case KEY_NUMPAD_DECIMAL: return VK_DECIMAL;

		case KEY_STOP_MEDIA: return VK_MEDIA_STOP;
		case KEY_PREV_TRACK: return VK_MEDIA_PREV_TRACK;
		case KEY_PLAY_PAUSE: return VK_MEDIA_PLAY_PAUSE;
		case KEY_NEXT_TRACK: return VK_MEDIA_NEXT_TRACK;

		case KEY_F13: return VK_F13;
		case KEY_F14: return VK_F14;
		case KEY_F15: return VK_F15;
		case KEY_F16: return VK_F16;
		case KEY_F17: return VK_F17;
		case KEY_F18: return VK_F18;
		case KEY_F19: return VK_F19;
		case KEY_F20: return VK_F20;
		case KEY_F21: return VK_F21;
		case KEY_F22: return VK_F22;
		}
		return 0;
	}

	int soup_key_to_translated_virtual_key(uint8_t key) noexcept
	{
		// MapVirtualKeyA would return VK_INSERT instead of VK_NUMPAD0 etc.
		switch (key)
		{
		case KEY_NUMPAD0: return VK_NUMPAD0;
		case KEY_NUMPAD1: return VK_NUMPAD1;
		case KEY_NUMPAD2: return VK_NUMPAD2;
		case KEY_NUMPAD3: return VK_NUMPAD3;
		case KEY_NUMPAD4: return VK_NUMPAD4;
		case KEY_NUMPAD5: return VK_NUMPAD5;
		case KEY_NUMPAD6: return VK_NUMPAD6;
		case KEY_NUMPAD7: return VK_NUMPAD7;
		case KEY_NUMPAD8: return VK_NUMPAD8;
		case KEY_NUMPAD9: return VK_NUMPAD9;
		case KEY_NUMPAD_DECIMAL: return VK_DECIMAL;
		default:;
		}
		return MapVirtualKeyA(soup_key_to_ps2_scancode(key), MAPVK_VSC_TO_VK_EX);
	}

	Key virtual_key_to_soup_key(int vk) noexcept
	{
		return ps2_scancode_to_soup_key(static_cast<uint16_t>(MapVirtualKeyA(vk, MAPVK_VK_TO_VSC_EX)));
	}
#endif

	uint16_t soup_key_to_ps2_scancode(uint8_t key) noexcept
	{
		switch (key)
		{
		case KEY_ESCAPE: return PS2_ESCAPE;
		case KEY_F1: return PS2_F1;
		case KEY_F2: return PS2_F2;
		case KEY_F3: return PS2_F3;
		case KEY_F4: return PS2_F4;
		case KEY_F5: return PS2_F5;
		case KEY_F6: return PS2_F6;
		case KEY_F7: return PS2_F7;
		case KEY_F8: return PS2_F8;
		case KEY_F9: return PS2_F9;
		case KEY_F10: return PS2_F10;
		case KEY_F11: return PS2_F11;
		case KEY_F12: return PS2_F12;

		case KEY_BACKQUOTE: return PS2_BACKQUOTE;
		case KEY_1: return PS2_1;
		case KEY_2: return PS2_2;
		case KEY_3: return PS2_3;
		case KEY_4: return PS2_4;
		case KEY_5: return PS2_5;
		case KEY_6: return PS2_6;
		case KEY_7: return PS2_7;
		case KEY_8: return PS2_8;
		case KEY_9: return PS2_9;
		case KEY_0: return PS2_0;
		case KEY_MINUS: return PS2_MINUS;
		case KEY_EQUALS: return PS2_EQUALS;
		case KEY_BACKSPACE: return PS2_BACKSPACE;
		case KEY_TAB: return PS2_TAB;
		case KEY_Q: return PS2_Q;
		case KEY_W: return PS2_W;
		case KEY_E: return PS2_E;
		case KEY_R: return PS2_R;
		case KEY_T: return PS2_T;
		case KEY_Y: return PS2_Y;
		case KEY_U: return PS2_U;
		case KEY_I: return PS2_I;
		case KEY_O: return PS2_O;
		case KEY_P: return PS2_P;
		case KEY_BRACKET_LEFT: return PS2_BRACKET_LEFT;
		case KEY_BRACKET_RIGHT: return PS2_BRACKET_RIGHT;
		case KEY_ENTER: return PS2_ENTER;
		case KEY_CAPS_LOCK: return PS2_CAPS_LOCK;
		case KEY_A: return PS2_A;
		case KEY_S: return PS2_S;
		case KEY_D: return PS2_D;
		case KEY_F: return PS2_F;
		case KEY_G: return PS2_G;
		case KEY_H: return PS2_H;
		case KEY_J: return PS2_J;
		case KEY_K: return PS2_K;
		case KEY_L: return PS2_L;
		case KEY_SEMICOLON: return PS2_SEMICOLON;
		case KEY_QUOTE: return PS2_QUOTE;
		case KEY_BACKSLASH: return PS2_BACKSLASH;
		case KEY_LSHIFT: return PS2_LSHIFT;
		case KEY_INTL_BACKSLASH: return PS2_INTL_BACKSLASH;
		case KEY_Z: return PS2_Z;
		case KEY_X: return PS2_X;
		case KEY_C: return PS2_C;
		case KEY_V: return PS2_V;
		case KEY_B: return PS2_B;
		case KEY_N: return PS2_N;
		case KEY_M: return PS2_M;
		case KEY_COMMA: return PS2_COMMA;
		case KEY_PERIOD: return PS2_PERIOD;
		case KEY_SLASH: return PS2_SLASH;
		case KEY_RSHIFT: return PS2_RSHIFT;
		case KEY_LCTRL: return PS2_LCTRL;
		case KEY_LMETA: return PS2_LMETA;
		case KEY_LALT: return PS2_LALT;
		case KEY_SPACE: return PS2_SPACE;
		case KEY_RALT: return PS2_RALT;
		case KEY_RMETA: return PS2_RMETA;
			//case KEY_FN: return PS2_FN;
		case KEY_RCTRL: return PS2_RCTRL;

		case KEY_PRINT_SCREEN: return PS2_PRINT_SCREEN;
		case KEY_PAUSE: return PS2_PAUSE;
		case KEY_SCROLL_LOCK: return PS2_SCROLL_LOCK;

		case KEY_INSERT: return PS2_INSERT;
		case KEY_HOME: return PS2_HOME;
		case KEY_PAGE_UP: return PS2_PAGE_UP;
		case KEY_DEL: return PS2_DEL;
		case KEY_END: return PS2_END;
		case KEY_PAGE_DOWN: return PS2_PAGE_DOWN;

		case KEY_ARROW_UP: return PS2_ARROW_UP;
		case KEY_ARROW_LEFT: return PS2_ARROW_LEFT;
		case KEY_ARROW_DOWN: return PS2_ARROW_DOWN;
		case KEY_ARROW_RIGHT: return PS2_ARROW_RIGHT;

		case KEY_NUM_LOCK: return PS2_NUM_LOCK;
		case KEY_NUMPAD_DIVIDE: return PS2_NUMPAD_DIVIDE;
		case KEY_NUMPAD_MULTIPLY: return PS2_NUMPAD_MULTIPLY;
		case KEY_NUMPAD_SUBTRACT: return PS2_NUMPAD_SUBTRACT;
		case KEY_NUMPAD7: return PS2_NUMPAD7;
		case KEY_NUMPAD8: return PS2_NUMPAD8;
		case KEY_NUMPAD9: return PS2_NUMPAD9;
		case KEY_NUMPAD_ADD: return PS2_NUMPAD_ADD;
		case KEY_NUMPAD4: return PS2_NUMPAD4;
		case KEY_NUMPAD5: return PS2_NUMPAD5;
		case KEY_NUMPAD6: return PS2_NUMPAD6;
		case KEY_NUMPAD1: return PS2_NUMPAD1;
		case KEY_NUMPAD2: return PS2_NUMPAD2;
		case KEY_NUMPAD3: return PS2_NUMPAD3;
		case KEY_NUMPAD_ENTER: return PS2_NUMPAD_ENTER;
		case KEY_NUMPAD0: return PS2_NUMPAD0;
		case KEY_NUMPAD_DECIMAL: return PS2_NUMPAD_DECIMAL;

		case KEY_STOP_MEDIA: return PS2_STOP_MEDIA;
		case KEY_PREV_TRACK: return PS2_PREV_TRACK;
		case KEY_PLAY_PAUSE: return PS2_PLAY_PAUSE;
		case KEY_NEXT_TRACK: return PS2_NEXT_TRACK;

		case KEY_F13: return PS2_F13;
		case KEY_F14: return PS2_F14;
		case KEY_F15: return PS2_F15;
		}
		return 0;
	}

	Key ps2_scancode_to_soup_key(uint16_t scancode) noexcept
	{
		switch (scancode)
		{
		case PS2_ESCAPE: return KEY_ESCAPE;
		case PS2_F1: return KEY_F1;
		case PS2_F2: return KEY_F2;
		case PS2_F3: return KEY_F3;
		case PS2_F4: return KEY_F4;
		case PS2_F5: return KEY_F5;
		case PS2_F6: return KEY_F6;
		case PS2_F7: return KEY_F7;
		case PS2_F8: return KEY_F8;
		case PS2_F9: return KEY_F9;
		case PS2_F10: return KEY_F10;
		case PS2_F11: return KEY_F11;
		case PS2_F12: return KEY_F12;

		case PS2_BACKQUOTE: return KEY_BACKQUOTE;
		case PS2_1: return KEY_1;
		case PS2_2: return KEY_2;
		case PS2_3: return KEY_3;
		case PS2_4: return KEY_4;
		case PS2_5: return KEY_5;
		case PS2_6: return KEY_6;
		case PS2_7: return KEY_7;
		case PS2_8: return KEY_8;
		case PS2_9: return KEY_9;
		case PS2_0: return KEY_0;
		case PS2_MINUS: return KEY_MINUS;
		case PS2_EQUALS: return KEY_EQUALS;
		case PS2_BACKSPACE: return KEY_BACKSPACE;
		case PS2_TAB: return KEY_TAB;
		case PS2_Q: return KEY_Q;
		case PS2_W: return KEY_W;
		case PS2_E: return KEY_E;
		case PS2_R: return KEY_R;
		case PS2_T: return KEY_T;
		case PS2_Y: return KEY_Y;
		case PS2_U: return KEY_U;
		case PS2_I: return KEY_I;
		case PS2_O: return KEY_O;
		case PS2_P: return KEY_P;
		case PS2_BRACKET_LEFT: return KEY_BRACKET_LEFT;
		case PS2_BRACKET_RIGHT: return KEY_BRACKET_RIGHT;
		case PS2_ENTER: return KEY_ENTER;
		case PS2_CAPS_LOCK: return KEY_CAPS_LOCK;
		case PS2_A: return KEY_A;
		case PS2_S: return KEY_S;
		case PS2_D: return KEY_D;
		case PS2_F: return KEY_F;
		case PS2_G: return KEY_G;
		case PS2_H: return KEY_H;
		case PS2_J: return KEY_J;
		case PS2_K: return KEY_K;
		case PS2_L: return KEY_L;
		case PS2_SEMICOLON: return KEY_SEMICOLON;
		case PS2_QUOTE: return KEY_QUOTE;
		case PS2_BACKSLASH: return KEY_BACKSLASH;
		case PS2_LSHIFT: return KEY_LSHIFT;
		case PS2_INTL_BACKSLASH: return KEY_INTL_BACKSLASH;
		case PS2_Z: return KEY_Z;
		case PS2_X: return KEY_X;
		case PS2_C: return KEY_C;
		case PS2_V: return KEY_V;
		case PS2_B: return KEY_B;
		case PS2_N: return KEY_N;
		case PS2_M: return KEY_M;
		case PS2_COMMA: return KEY_COMMA;
		case PS2_PERIOD: return KEY_PERIOD;
		case PS2_SLASH: return KEY_SLASH;
		case PS2_RSHIFT: return KEY_RSHIFT;
		case PS2_LCTRL: return KEY_LCTRL;
		case PS2_LMETA: return KEY_LMETA;
		case PS2_LALT: return KEY_LALT;
		case PS2_SPACE: return KEY_SPACE;
		case PS2_RALT: return KEY_RALT;
		case PS2_RMETA: return KEY_RMETA;
			//case PS2_FN: return KEY_FN;
		case PS2_RCTRL: return KEY_RCTRL;

		case PS2_PRINT_SCREEN: return KEY_PRINT_SCREEN;
		case PS2_PAUSE: return KEY_PAUSE;
		case PS2_SCROLL_LOCK: return KEY_SCROLL_LOCK;

		case PS2_INSERT: return KEY_INSERT;
		case PS2_HOME: return KEY_HOME;
		case PS2_PAGE_UP: return KEY_PAGE_UP;
		case PS2_DEL: return KEY_DEL;
		case PS2_END: return KEY_END;
		case PS2_PAGE_DOWN: return KEY_PAGE_DOWN;

		case PS2_ARROW_UP: return KEY_ARROW_UP;
		case PS2_ARROW_LEFT: return KEY_ARROW_LEFT;
		case PS2_ARROW_DOWN: return KEY_ARROW_DOWN;
		case PS2_ARROW_RIGHT: return KEY_ARROW_RIGHT;

		case PS2_NUM_LOCK: return KEY_NUM_LOCK;
		case PS2_NUMPAD_DIVIDE: return KEY_NUMPAD_DIVIDE;
		case PS2_NUMPAD_MULTIPLY: return KEY_NUMPAD_MULTIPLY;
		case PS2_NUMPAD_SUBTRACT: return KEY_NUMPAD_SUBTRACT;
		case PS2_NUMPAD7: return KEY_NUMPAD7;
		case PS2_NUMPAD8: return KEY_NUMPAD8;
		case PS2_NUMPAD9: return KEY_NUMPAD9;
		case PS2_NUMPAD_ADD: return KEY_NUMPAD_ADD;
		case PS2_NUMPAD4: return KEY_NUMPAD4;
		case PS2_NUMPAD5: return KEY_NUMPAD5;
		case PS2_NUMPAD6: return KEY_NUMPAD6;
		case PS2_NUMPAD1: return KEY_NUMPAD1;
		case PS2_NUMPAD2: return KEY_NUMPAD2;
		case PS2_NUMPAD3: return KEY_NUMPAD3;
		case PS2_NUMPAD_ENTER: return KEY_NUMPAD_ENTER;
		case PS2_NUMPAD0: return KEY_NUMPAD0;
		case PS2_NUMPAD_DECIMAL: return KEY_NUMPAD_DECIMAL;

		case PS2_STOP_MEDIA: return KEY_STOP_MEDIA;
		case PS2_PREV_TRACK: return KEY_PREV_TRACK;
		case PS2_PLAY_PAUSE: return KEY_PLAY_PAUSE;
		case PS2_NEXT_TRACK: return KEY_NEXT_TRACK;

		case PS2_F13: return KEY_F13;
		case PS2_F14: return KEY_F14;
		case PS2_F15: return KEY_F15;
		}
		return KEY_NONE;
	}
}
