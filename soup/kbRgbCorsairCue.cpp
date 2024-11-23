#include "kbRgbCorsairCue.hpp"
#if SOUP_WINDOWS

#include "hwHid.hpp"
#include "json.hpp"
#include "StringWriter.hpp"
#include "os.hpp"

NAMESPACE_SOUP
{
	bool kbRgbCorsairCue::ensureConnected()
	{
		if (pipe.isConnected())
		{
			return true;
		}
		if (pipe.connect("iCUESDKv4"))
		{
			auto params = soup::make_unique<JsonObject>();
			params->add("clientVersion", "4.0.12");
			params->add("pid", os::getProcessId());

			JsonObject root;
			root.add("code", -1);
			root.add("method", 0);
			root.add("params", std::move(params));

			send(root.encode());
			SOUP_UNUSED(receive());

			return true;
		}
		return false;
	}

	void kbRgbCorsairCue::send(const std::string& data)
	{
		//std::cout << "[kbRgbCorsairCue] Send: " << data << "\n";
		StringWriter sw(false);
		sw.str_lp<u32be_t>(data);
		pipe.write(sw.data);
	}

	std::string kbRgbCorsairCue::receive()
	{
		SOUP_UNUSED(pipe.read()); // ignore length prefix
		std::string data = pipe.read();
		//std::cout << "[kbRgbCorsairCue] Receive: " << data << "\n";
		return data;
	}

	bool kbRgbCorsairCue::controlsDevice(const hwHid& hid) const noexcept
	{
		return hid.vendor_id == 0x1b1c;
	}

	void kbRgbCorsairCue::deinit()
	{
		// There doesn't seem to be any way to reset keys for a specific device, so...
		pipe.close();
	}

	// https://github.com/CorsairOfficial/cue-sdk/blob/master/src/include/iCUESDKLedIdEnum.h
	[[nodiscard]] static int soup_key_to_corsair_led_id(Key key)
	{
		switch (key)
		{
		case KEY_ESCAPE: return 1;
		case KEY_F1: return 2;
		case KEY_F2: return 3;
		case KEY_F3: return 4;
		case KEY_F4: return 5;
		case KEY_F5: return 6;
		case KEY_F6: return 7;
		case KEY_F7: return 8;
		case KEY_F8: return 9;
		case KEY_F9: return 10;
		case KEY_F10: return 11;
		case KEY_F11: return 12;
		case KEY_F12: return 13;
		case KEY_BACKQUOTE: return 14;
		case KEY_1: return 15;
		case KEY_2: return 16;
		case KEY_3: return 17;
		case KEY_4: return 18;
		case KEY_5: return 19;
		case KEY_6: return 20;
		case KEY_7: return 21;
		case KEY_8: return 22;
		case KEY_9: return 23;
		case KEY_0: return 24;
		case KEY_MINUS: return 25;
		case KEY_EQUALS: return 26;
		case KEY_BACKSPACE: return 27;
		case KEY_TAB: return 28;
		case KEY_Q: return 29;
		case KEY_W: return 30;
		case KEY_E: return 31;
		case KEY_R: return 32;
		case KEY_T: return 33;
		case KEY_Y: return 34;
		case KEY_U: return 35;
		case KEY_I: return 36;
		case KEY_O: return 37;
		case KEY_P: return 38;
		case KEY_BRACKET_LEFT: return 39;
		case KEY_BRACKET_RIGHT: return 40;
		case KEY_ENTER: return 54;
		case KEY_CAPS_LOCK: return 41;
		case KEY_A: return 42;
		case KEY_S: return 43;
		case KEY_D: return 44;
		case KEY_F: return 45;
		case KEY_G: return 46;
		case KEY_H: return 47;
		case KEY_J: return 48;
		case KEY_K: return 49;
		case KEY_L: return 50;
		case KEY_SEMICOLON: return 51;
		case KEY_QUOTE: return 52;
		case KEY_BACKSLASH: return 97; //return 53;
		case KEY_LSHIFT: return 55;
		case KEY_INTL_BACKSLASH: return 56;
		case KEY_Z: return 57;
		case KEY_X: return 58;
		case KEY_C: return 59;
		case KEY_V: return 60;
		case KEY_B: return 61;
		case KEY_N: return 62;
		case KEY_M: return 63;
		case KEY_COMMA: return 64;
		case KEY_PERIOD: return 65;
		case KEY_SLASH: return 66;
		case KEY_RSHIFT: return 67;
		case KEY_LCTRL: return 68;
		case KEY_LMETA: return 69;
		case KEY_LALT: return 70;
		case KEY_SPACE: return 71;
		case KEY_RALT: return 72;
		case KEY_RMETA: return 73;
		case KEY_FN: return 129;
		case KEY_CTX: return 74;
		case KEY_RCTRL: return 75;
		case KEY_PRINT_SCREEN: return 84;
		case KEY_PAUSE: return 86;
		case KEY_SCROLL_LOCK: return 85;
		case KEY_INSERT: return 87;
		case KEY_HOME: return 88;
		case KEY_PAGE_UP: return 89;
		case KEY_DEL: return 90;
		case KEY_END: return 91;
		case KEY_PAGE_DOWN: return 92;
		case KEY_ARROW_UP: return 93;
		case KEY_ARROW_LEFT: return 94;
		case KEY_ARROW_DOWN: return 95;
		case KEY_ARROW_RIGHT: return 96;
		case KEY_NUM_LOCK: return 105;
		case KEY_NUMPAD_DIVIDE: return 106;
		case KEY_NUMPAD_MULTIPLY: return 107;
		case KEY_NUMPAD_SUBTRACT: return 108;
		case KEY_NUMPAD7: return 109;
		case KEY_NUMPAD8: return 110;
		case KEY_NUMPAD9: return 111;
		case KEY_NUMPAD_ADD: return 112;
		case KEY_NUMPAD4: return 113;
		case KEY_NUMPAD5: return 114;
		case KEY_NUMPAD6: return 115;
		case KEY_NUMPAD1: return 116;
		case KEY_NUMPAD2: return 117;
		case KEY_NUMPAD3: return 118;
		case KEY_NUMPAD_ENTER: return 120;
		case KEY_NUMPAD0: return 121;
		case KEY_NUMPAD_DECIMAL: return 122;
		case KEY_STOP_MEDIA: return 101;
		case KEY_PREV_TRACK: return 102;
		case KEY_PLAY_PAUSE: return 103;
		case KEY_NEXT_TRACK: return 104;
		case KEY_OEM_1: return 0x30001; // Profile switch key
		case KEY_OEM_2: return 98; // Brightness
		case KEY_OEM_3: return 99; // Winlock
		case KEY_OEM_4: return 100; // Mute
		default:;
		}
		return 0;
	}

	[[nodiscard]] static uint32_t encodeColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
	{
		return (a << 24) | (r << 16) | (g << 8) | b;
	}

	static void addLedToArray(JsonArray& arr, int led_id, Rgb colour)
	{
		auto led = soup::make_unique<JsonObject>();
		led->add("id", led_id);
		led->add("value", encodeColour(colour.r, colour.g, colour.b, 255));
		arr.children.emplace_back(std::move(led));
	}

	void kbRgbCorsairCue::submitColours(UniquePtr<JsonArray>&& ledColors)
	{
		if (ensureConnected())
		{
			auto params = soup::make_unique<JsonObject>();
			params->add("deviceId", id);
			params->add("ledColors", std::move(ledColors));

			JsonObject root;
			root.add("method", 10);
			root.add("code", 3);
			root.add("params", std::move(params));

			send(root.encode());
			SOUP_UNUSED(receive());
		}
	}

	void kbRgbCorsairCue::setKey(Key key, Rgb colour)
	{
		if (auto led_id = soup_key_to_corsair_led_id(key))
		{
			auto ledColors = soup::make_unique<JsonArray>();
			addLedToArray(*ledColors, led_id, colour);
			if (led_id == 71)
			{
				// Also update spacebar left & right
				addLedToArray(*ledColors, 0x30002, colour);
				addLedToArray(*ledColors, 0x30003, colour);
			}
			submitColours(std::move(ledColors));
		}
	}

	void kbRgbCorsairCue::setKeys(const Rgb(&colours)[NUM_KEYS])
	{
		auto ledColors = soup::make_unique<JsonArray>();
		for (uint8_t key = 0; key != NUM_KEYS; ++key)
		{
			if (auto led_id = soup_key_to_corsair_led_id(static_cast<Key>(key)))
			{
				addLedToArray(*ledColors, led_id, colours[key]);
			}
		}

		// Also update spacebar left & right
		addLedToArray(*ledColors, 0x30002, colours[KEY_SPACE]);
		addLedToArray(*ledColors, 0x30003, colours[KEY_SPACE]);

		submitColours(std::move(ledColors));
	}

	void kbRgbCorsairCue::setAllKeys(Rgb colour)
	{
		auto ledColors = soup::make_unique<JsonArray>();
		for (int led_id = 1; led_id != 130; ++led_id)
		{
			addLedToArray(*ledColors, led_id, colour);
		}
		addLedToArray(*ledColors, 0x30001, colour); // Profile switch key
		addLedToArray(*ledColors, 0x30002, colour); // Spacebar left
		addLedToArray(*ledColors, 0x30003, colour); // Spacebar right
		addLedToArray(*ledColors, 0x30004, colour); // Corsair logo left
		addLedToArray(*ledColors, 0x30005, colour); // Corsair logo right
		submitColours(std::move(ledColors));
	}

	uint8_t kbRgbCorsairCue::getNumColumns() const noexcept
	{
		return 21;
	}

	Key kbRgbCorsairCue::mapPosToKey(uint8_t row, uint8_t column) const noexcept
	{
		if (row == 0)
		{
			switch (column)
			{
			case 0: return KEY_ESCAPE;
			case 2: return KEY_F1;
			case 3: return KEY_F2;
			case 4: return KEY_F3;
			case 5: return KEY_F4;
			case 6: return KEY_F5;
			case 7: return KEY_F6;
			case 8: return KEY_F7;
			case 9: return KEY_F8;
			case 10: return KEY_F9;
			case 11: return KEY_F10;
			case 12: return KEY_F11;
			case 13: return KEY_F12;
			case 14: return KEY_PRINT_SCREEN;
			case 15: return KEY_PAUSE;
			case 16: return KEY_SCROLL_LOCK;
			case 17: return KEY_STOP_MEDIA;
			case 18: return KEY_PREV_TRACK;
			case 19: return KEY_PLAY_PAUSE;
			case 20: return KEY_NEXT_TRACK;
			}
		}
		else if (row == 1)
		{
			switch (column)
			{
			case 0: return KEY_BACKQUOTE;
			case 1: return KEY_1;
			case 2: return KEY_2;
			case 3: return KEY_3;
			case 4: return KEY_4;
			case 5: return KEY_5;
			case 6: return KEY_6;
			case 7: return KEY_7;
			case 8: return KEY_8;
			case 9: return KEY_9;
			case 10: return KEY_0;
			case 11: return KEY_MINUS;
			case 12: return KEY_EQUALS;
			case 13: return KEY_BACKSPACE;
			case 14: return KEY_INSERT;
			case 15: return KEY_HOME;
			case 16: return KEY_PAGE_UP;
			case 17: return KEY_NUM_LOCK;
			case 18: return KEY_NUMPAD_DIVIDE;
			case 19: return KEY_NUMPAD_MULTIPLY;
			case 20: return KEY_NUMPAD_SUBTRACT;
			}
		}
		else if (row == 2)
		{
			switch (column)
			{
			case 0: return KEY_TAB;
			case 1: return KEY_Q;
			case 2: return KEY_W;
			case 3: return KEY_E;
			case 4: return KEY_R;
			case 5: return KEY_T;
			case 6: return KEY_Y;
			case 7: return KEY_U;
			case 8: return KEY_I;
			case 9: return KEY_O;
			case 10: return KEY_P;
			case 11: return KEY_BRACKET_LEFT;
			case 12: return KEY_BRACKET_RIGHT;
			case 13: return KEY_BACKSLASH; // ANSI
			case 14: return KEY_DEL;
			case 15: return KEY_END;
			case 16: return KEY_PAGE_DOWN;
			case 17: return KEY_NUMPAD7;
			case 18: return KEY_NUMPAD8;
			case 19: return KEY_NUMPAD9;
			case 20: return KEY_NUMPAD_ADD;
			}
		}
		else if (row == 3)
		{
			switch (column)
			{
			case 0: return KEY_CAPS_LOCK;
			case 1: return KEY_A;
			case 2: return KEY_S;
			case 3: return KEY_D;
			case 4: return KEY_F;
			case 5: return KEY_G;
			case 6: return KEY_H;
			case 7: return KEY_J;
			case 8: return KEY_K;
			case 9: return KEY_L;
			case 10: return KEY_SEMICOLON;
			case 11: return KEY_QUOTE;
			case 12: return KEY_BACKSLASH; // ISO
			case 13: return KEY_ENTER;
			case 17: return KEY_NUMPAD4;
			case 18: return KEY_NUMPAD5;
			case 19: return KEY_NUMPAD6;
			}
		}
		else if (row == 4)
		{
			switch (column)
			{
			case 0: return KEY_LSHIFT;
			case 1: return KEY_INTL_BACKSLASH; // ISO
			case 2: return KEY_Z;
			case 3: return KEY_X;
			case 4: return KEY_C;
			case 5: return KEY_V;
			case 6: return KEY_B;
			case 7: return KEY_N;
			case 8: return KEY_M;
			case 9: return KEY_COMMA;
			case 10: return KEY_PERIOD;
			case 11: return KEY_SLASH;
			case 13: return KEY_RSHIFT;
			case 15: return KEY_ARROW_UP;
			case 17: return KEY_NUMPAD1;
			case 18: return KEY_NUMPAD2;
			case 19: return KEY_NUMPAD3;
			case 20: return KEY_NUMPAD_ENTER;
			}
		}
		else if (row == 5)
		{
			switch (column)
			{
			case 0: return KEY_LCTRL;
			case 1: return KEY_LMETA;
			case 2: return KEY_LALT;
			case 6: return KEY_SPACE;
			case 10: return KEY_RALT;
			case 11: return KEY_FN;
			case 12: return KEY_CTX;
			case 13: return KEY_RCTRL;
			case 14: return KEY_ARROW_LEFT;
			case 15: return KEY_ARROW_DOWN;
			case 16: return KEY_ARROW_RIGHT;
			case 18: return KEY_NUMPAD0;
			case 19: return KEY_NUMPAD_DECIMAL;
			}
		}
		return KEY_NONE;
	}
}

/* A little dummy server to test the SDK against:

#include <IpcSocket.hpp>
#include <json.hpp>
#include <Thread.hpp>

#include "iCUESDK.h"
#include "iCUESDKLedIdEnum.h"
#pragma comment(lib, "iCUESDK.x64_2019.lib")

using namespace soup;

static std::string Pack(const std::string& data)
{
	StringWriter sw(false);
	sw.str_lp_u32(data);
	return sw.data;
}

int main()
{
	Thread t([]
	{
		IpcSocket sock;
		if (sock.bind("iCUESDKv4"))
		{
			while (sock.accept())
			{
				std::cout << "accept\n";
				while (true)
				{
					auto data = sock.read();
					if (data.empty())
					{
						break;
					}
					data.erase(0, 4); // ignore length prefix
					std::cout << "read: " << data << "\n";
					auto jr = json::decode(data);
					if (jr->asObj().at("method").asInt() == 0)
					{
						sock.write(Pack(R"({"code":-1,"error":0,"method":0,"result":{"serverHostVersion":"5.7.106","serverVersion":"4.0.18","state":6}})"));
					}
					else if (jr->asObj().at("method").asInt() == 1)
					{
						sock.write(Pack(R"({"code":2,"error":0,"method":1,"result":{"deviceInfos":[{"channelCount":0,"id":"{3f8b4d27-ad0e-459c-9987-5d769a8c2523}","ledCount":117,"model":"K70 MAX","serial":"DEEZNUTS","type":1}]}})"));
					}
					else if (jr->asObj().at("method").asInt() == 10)
					{
						sock.write(Pack(R"({"code":3,"error":0,"method":10,"result":{}})"));
					}
				}
				sock.disconnect();
			}
		}
	});

	CorsairConnect([](void* ctx, const CorsairSessionStateChanged* eventData)
	{
		std::cout << "new session state: " << (int)eventData->state << "\n";
		if (eventData->state == CSS_Connected)
		{
			CorsairDeviceFilter filter;
			filter.deviceTypeMask = CDT_Keyboard;

			CorsairDeviceInfo kbds[1];
			int size;

			CorsairGetDevices(&filter, 1, kbds, &size);

			std::cout << size << " keyboards, " << kbds[0].model << "\n";

			CorsairLedColor led;
			led.id = CLK_Escape;
			led.r = 1;
			led.g = 2;
			led.b = 3;
			led.a = 255;
			CorsairSetLedColors(kbds[0].id, 1, &led);

			std::cout << "leds set\n";
		}
	}, nullptr);

	while (true);
}
*/

#endif
