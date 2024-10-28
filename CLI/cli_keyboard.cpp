#include "cli.hpp"

#include <iostream>

#include <AnalogueKeyboard.hpp>
#include <DigitalKeyboard.hpp>
#include <os.hpp>
#include <RasterFont.hpp>
#include <RenderTarget.hpp>
#include <Rgb.hpp>
#include <Thread.hpp>
#include <visKeyboard.hpp>
#include <Window.hpp>

#define FORCE_DIGITAL false

using namespace soup;

static bool running = true;
static std::string kbd_name = "Digital Keyboard";
static Window w;
static visKeyboard viskbd;
static AnalogueKeyboard* analogue_kbd = nullptr;

void cli_keyboard()
{
#if (SOUP_WINDOWS || SOUP_LINUX) && !SOUP_CROSS_COMPILE
	Thread t([](Capture&&)
	{
		// Wait until window is created
		while (!w)
		{
			os::sleep(1);
		}

		DigitalKeyboard digital_kbd;

		while (running)
		{
			if (analogue_kbd)
			{
				SOUP_IF_UNLIKELY (analogue_kbd->disconnected)
				{
					std::cout << analogue_kbd->name << " disconnected, switching back to digital mode.\n";

					delete analogue_kbd;
					analogue_kbd = nullptr;

					viskbd.has_ctx_key = false;
					kbd_name = "Digital Keyboard";
					w.redraw();
				}
				else
				{
					auto keys = analogue_kbd->getActiveKeys();
					viskbd.clear();
					for (const auto& key : keys)
					{
						//std::cout << std::hex << "HID-SC " << (int)key.getHidScancode() << ", SK " << (int)key.getSoupKey() << ", VK " << key.getVk() << ", VKT " << key.getVkTranslated() << " - " << key.getFValue() << "\n";
						if (auto sk = key.getSoupKey(); sk != KEY_NONE)
						{
							viskbd.values[sk] = static_cast<uint8_t>(key.fvalue * 255.0f);
						}
					}
					w.redraw();
				}
			}
			else
			{
				std::vector<AnalogueKeyboard> analogue_kbds{};
#if !FORCE_DIGITAL
				analogue_kbds = AnalogueKeyboard::getAll();
#endif
				if (analogue_kbds.empty())
				{
					digital_kbd.update();

					if (digital_kbd.isActive())
					{
						uint8_t dblbuf[NUM_KEYS];
						for (auto i = 0; i != NUM_KEYS; ++i)
						{
							dblbuf[i] = digital_kbd.keys[i] ? 255 : 0;
						}

						if (memcmp(viskbd.values, dblbuf, sizeof(dblbuf)) != 0
							|| kbd_name != "Digital Keyboard"
							)
						{
							memcpy(viskbd.values, dblbuf, sizeof(dblbuf));
							kbd_name = "Digital Keyboard";
							w.redraw();
						}
					}
					else if (kbd_name == "Digital Keyboard")
					{
						kbd_name = "Can't Read Digital Keyboard";
						w.redraw();
					}

					os::sleep(10);
				}
				else
				{
					std::cout << "Found " << analogue_kbds.at(0).name << ", switching to analogue mode.\n";

					kbd_name = analogue_kbds.at(0).name;
					viskbd.has_ctx_key = analogue_kbds.at(0).has_ctx_key;
					w.redraw();

					analogue_kbd = new AnalogueKeyboard(std::move(analogue_kbds.at(0)));
				}
			}
		}

		if (analogue_kbd)
		{
			delete analogue_kbd;
		}
	});

	w = Window::create("Soup Keyboard Visualisation", visKeyboard::WIDTH * 5, (3 * 8) + 18 + visKeyboard::HEIGHT * 5);
	w.setDrawFunc([](Window, RenderTarget& rt)
	{
#if SOUP_WINDOWS
		rt.fill(Rgb::GREEN);
#else
		rt.fill(Rgb::GREY);
#endif
		rt.drawText(9, 9, kbd_name, RasterFont::simple8(), Rgb::WHITE, 3);
		viskbd.draw(rt, 0, (3 * 8) + 18, 5);
	});
#if SOUP_WINDOWS
	w.setInvisibleColour(Rgb::GREEN);
	w.setTopmost(true);
	w.setClickThrough(false);
#endif
	w.runMessageLoop();

	running = false;
	if (analogue_kbd)
	{
		analogue_kbd->hid.cancelReceiveReport();
	}
	t.awaitCompletion();
#else
	std::cout << "Sorry, this is currently not supported on your platform.\n";
#endif
}
