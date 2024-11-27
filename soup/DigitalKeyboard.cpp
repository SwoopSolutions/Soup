#include "DigitalKeyboard.hpp"

#if (SOUP_WINDOWS || SOUP_LINUX) && !SOUP_CROSS_COMPILE

#if SOUP_WINDOWS
	#pragma comment(lib, "dinput8.lib")
	#pragma comment(lib, "dxguid.lib")
#else
	#include "HidReportDescriptor.hpp"
	#include "HidScancode.hpp"
	#include "string.hpp"
#endif

NAMESPACE_SOUP
{
	void DigitalKeyboard::update() noexcept
	{
#if SOUP_WINDOWS
		if (!pDI)
		{
			if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&pDI, NULL)))
			{
				pDI = nullptr;
				return;
			}
		}

		if (!pKeyboard)
		{
			if (FAILED(pDI->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL)))
			{
				return;
			}
			if (FAILED(pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
			{
				pKeyboard->Release();
				pKeyboard = nullptr;
				return;
			}
			pKeyboard->SetCooperativeLevel(NULL, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
			pKeyboard->Acquire();
		}

		uint8_t state[256];
		if (SUCCEEDED(pKeyboard->GetDeviceState(sizeof(state), state)))
		{
			keys[KEY_ESCAPE] = state[DIK_ESCAPE];
			keys[KEY_F1] = state[DIK_F1];
			keys[KEY_F2] = state[DIK_F2];
			keys[KEY_F3] = state[DIK_F3];
			keys[KEY_F4] = state[DIK_F4];
			keys[KEY_F5] = state[DIK_F5];
			keys[KEY_F6] = state[DIK_F6];
			keys[KEY_F7] = state[DIK_F7];
			keys[KEY_F8] = state[DIK_F8];
			keys[KEY_F9] = state[DIK_F9];
			keys[KEY_F10] = state[DIK_F10];
			keys[KEY_F11] = state[DIK_F11];
			keys[KEY_F12] = state[DIK_F12];

			keys[KEY_BACKQUOTE] = state[DIK_GRAVE];
			keys[KEY_1] = state[DIK_1];
			keys[KEY_2] = state[DIK_2];
			keys[KEY_3] = state[DIK_3];
			keys[KEY_4] = state[DIK_4];
			keys[KEY_5] = state[DIK_5];
			keys[KEY_6] = state[DIK_6];
			keys[KEY_7] = state[DIK_7];
			keys[KEY_8] = state[DIK_8];
			keys[KEY_9] = state[DIK_9];
			keys[KEY_0] = state[DIK_0];
			keys[KEY_MINUS] = state[DIK_MINUS];
			keys[KEY_EQUALS] = state[DIK_EQUALS];
			keys[KEY_BACKSPACE] = state[DIK_BACK];
			keys[KEY_TAB] = state[DIK_TAB];
			keys[KEY_Q] = state[DIK_Q];
			keys[KEY_W] = state[DIK_W];
			keys[KEY_E] = state[DIK_E];
			keys[KEY_R] = state[DIK_R];
			keys[KEY_T] = state[DIK_T];
			keys[KEY_Y] = state[DIK_Y];
			keys[KEY_U] = state[DIK_U];
			keys[KEY_I] = state[DIK_I];
			keys[KEY_O] = state[DIK_O];
			keys[KEY_P] = state[DIK_P];
			keys[KEY_BRACKET_LEFT] = state[DIK_LBRACKET];
			keys[KEY_BRACKET_RIGHT] = state[DIK_RBRACKET];
			keys[KEY_CAPS_LOCK] = state[DIK_CAPITAL];
			keys[KEY_A] = state[DIK_A];
			keys[KEY_S] = state[DIK_S];
			keys[KEY_D] = state[DIK_D];
			keys[KEY_F] = state[DIK_F];
			keys[KEY_G] = state[DIK_G];
			keys[KEY_H] = state[DIK_H];
			keys[KEY_J] = state[DIK_J];
			keys[KEY_K] = state[DIK_K];
			keys[KEY_L] = state[DIK_L];
			keys[KEY_SEMICOLON] = state[DIK_SEMICOLON];
			keys[KEY_QUOTE] = state[DIK_APOSTROPHE];
			keys[KEY_BACKSLASH] = state[DIK_BACKSLASH];
			keys[KEY_ENTER] = state[DIK_RETURN];
			keys[KEY_LSHIFT] = state[DIK_LSHIFT];
			keys[KEY_Z] = state[DIK_Z];
			keys[KEY_X] = state[DIK_X];
			keys[KEY_C] = state[DIK_C];
			keys[KEY_V] = state[DIK_V];
			keys[KEY_B] = state[DIK_B];
			keys[KEY_N] = state[DIK_N];
			keys[KEY_M] = state[DIK_M];
			keys[KEY_COMMA] = state[DIK_COMMA];
			keys[KEY_PERIOD] = state[DIK_PERIOD];
			keys[KEY_SLASH] = state[DIK_SLASH];
			keys[KEY_RSHIFT] = state[DIK_RSHIFT];
			keys[KEY_LCTRL] = state[DIK_LCONTROL];
			keys[KEY_LMETA] = state[DIK_LWIN];
			keys[KEY_LALT] = state[DIK_LMENU];
			keys[KEY_SPACE] = state[DIK_SPACE];
			keys[KEY_RALT] = state[DIK_RMENU];
			keys[KEY_RMETA] = state[DIK_RWIN];
			keys[KEY_RCTRL] = state[DIK_RCONTROL];

			keys[KEY_PRINT_SCREEN] = GetAsyncKeyState(VK_SNAPSHOT) & 0x8000;
			keys[KEY_PAUSE] = state[DIK_PAUSE];
			keys[KEY_SCROLL_LOCK] = state[DIK_SCROLL];

			keys[KEY_INSERT] = state[DIK_INSERT];
			keys[KEY_HOME] = state[DIK_HOME];
			keys[KEY_PAGE_UP] = state[DIK_PRIOR];
			keys[KEY_DEL] = state[DIK_DELETE];
			keys[KEY_END] = state[DIK_END];
			keys[KEY_PAGE_DOWN] = state[DIK_NEXT];

			keys[KEY_ARROW_UP] = state[DIK_UPARROW];
			keys[KEY_ARROW_LEFT] = state[DIK_LEFTARROW];
			keys[KEY_ARROW_DOWN] = state[DIK_DOWNARROW];
			keys[KEY_ARROW_RIGHT] = state[DIK_RIGHTARROW];

			keys[KEY_NUM_LOCK] = state[DIK_NUMLOCK];
			keys[KEY_NUMPAD_DIVIDE] = state[DIK_DIVIDE];
			keys[KEY_NUMPAD_MULTIPLY] = state[DIK_MULTIPLY];
			keys[KEY_NUMPAD_SUBTRACT] = state[DIK_SUBTRACT];
			keys[KEY_NUMPAD7] = state[DIK_NUMPAD7];
			keys[KEY_NUMPAD8] = state[DIK_NUMPAD8];
			keys[KEY_NUMPAD9] = state[DIK_NUMPAD9];
			keys[KEY_NUMPAD_ADD] = state[DIK_ADD];
			keys[KEY_NUMPAD4] = state[DIK_NUMPAD4];
			keys[KEY_NUMPAD5] = state[DIK_NUMPAD5];
			keys[KEY_NUMPAD6] = state[DIK_NUMPAD6];
			keys[KEY_NUMPAD1] = state[DIK_NUMPAD1];
			keys[KEY_NUMPAD2] = state[DIK_NUMPAD2];
			keys[KEY_NUMPAD3] = state[DIK_NUMPAD3];
			keys[KEY_NUMPAD_ENTER] = state[DIK_NUMPADENTER];
			keys[KEY_NUMPAD0] = state[DIK_NUMPAD0];
			keys[KEY_NUMPAD_DECIMAL] = state[DIK_DECIMAL];

			keys[KEY_STOP_MEDIA] = state[DIK_STOP];
			keys[KEY_PREV_TRACK] = state[DIK_PREVTRACK];
			keys[KEY_PLAY_PAUSE] = state[DIK_PLAYPAUSE];
			keys[KEY_NEXT_TRACK] = state[DIK_NEXTTRACK];

			keys[KEY_F13] = state[DIK_F13];
			keys[KEY_F14] = state[DIK_F14];
			keys[KEY_F15] = state[DIK_F15];
		}
#else
		if (!hid.isValid())
		{
			for (auto& hid_ : hwHid::getAll())
			{
				if (hid_.havePermission()
					&& hid_.usage_page == 0x01 // Generic Desktop
					&& hid_.usage == 0x06 // Keyboard
					)
				{
					hid = std::move(hid_);
					break;
				}
			}
		}

		if (hid.isValid())
		{
			while (hid.hasReport())
			{
				const auto rawdesc = string::fromFile(hid.path + "/device/report_descriptor");
				const auto desc = HidReportDescriptor::parse(rawdesc.data(), rawdesc.size());
				const auto& report = hid.receiveReportWithoutReportId();
				const auto parsed_report = desc.parseInputReport(report.data(), report.size());
				memset(keys, 0, sizeof(keys));
				for (const auto& usage : parsed_report.active_selectors)
				{
					if (usage.getPage() == 0x07) // Keyboard
					{
						if (const auto sk = hid_scancode_to_soup_key(usage.getId()); sk != KEY_NONE)
						{
							keys[sk] = true;
						}
					}
				}
			}
		}
#endif
	}

	void DigitalKeyboard::deinit() noexcept
	{
#if SOUP_WINDOWS
		if (pKeyboard)
		{
			pKeyboard->Unacquire();
			pKeyboard->Release();
			pKeyboard = nullptr;
		}
		if (pDI)
		{
			pDI->Release();
			pDI = nullptr;
		}
#else
		hid.reset();
#endif
	}
}

#endif
