#include "kbRgbRazerChroma.hpp"
#if !SOUP_WASM

#include "hwHid.hpp"
#include "json.hpp"
#include "os.hpp"

NAMESPACE_SOUP
{
	bool kbRgbRazerChroma::isAvailable()
	{
		return Socket::isPortLocallyBound(54235);
	}

	void kbRgbRazerChroma::updateMaintainTask()
	{
		if (task
			&& task->isWorkDone()
			)
		{
			task.reset();
		}
	}

	void kbRgbRazerChroma::ensureMaintainTask()
	{
		SharedPtr<JsonObject> payload;
		if (task
			&& task->isWorkDone()
			)
		{
			payload = std::move(task->payload);
			task.reset();
		}
		if (!task)
		{
			std::string base;
			do
			{
				HttpRequest req(Uri("http://localhost:54235/razer/chromasdk"));
				req.addHeader("Content-Type: application/json");
				req.setPayload(R"EOC({
	"title": "Soup",
	"description": "Soup is a powerful framework for programmers.",
	"author": {
		"name": "Calamity, Inc.",
		"contact": "https://calamity.inc"
	},
	"device_supported": [
		"keyboard"
	],
	"category": "application"
})EOC");
				auto resp = req.execute();
				auto jr = json::decode(resp->body);
				if (jr->isObj())
				{
					base = jr->asObj().at("uri").asStr();
				}
				os::sleep(50);
			} while (base.empty() || !Socket::isPortLocallyBound(Uri(base).port));

			task = sched.add<MaintainTask>(std::move(base));
			task->payload = std::move(payload);
		}
	}

	kbRgbRazerChroma::MaintainTask& kbRgbRazerChroma::getMaintainTask()
	{
		ensureMaintainTask();
		return *task;
	}

	void kbRgbRazerChroma::commitColours()
	{
		auto payload = soup::make_shared<JsonObject>();
		payload->add("effect", "CHROMA_CUSTOM");

		auto param = soup::make_unique<JsonArray>();
		param->children.reserve(6);
		for (uint8_t row = 0; row != 6; ++row)
		{
			auto jrow = soup::make_unique<JsonArray>();
			jrow->children.reserve(22);
			for (uint8_t column = 0; column != 22; ++column)
			{
				jrow->children.emplace_back(soup::make_unique<JsonInt>(colours[row * 22 + column]));
			}
			param->children.emplace_back(std::move(jrow));
		}
		payload->add("param", std::move(param));

		getMaintainTask().payload = std::move(payload);
	}

	void kbRgbRazerChroma::MaintainTask::onTick()
	{
		if (!hrt.has_value())
		{
			if (!deinit_requested)
			{
				auto payload{ this->payload };
				if (payload)
				{
					hrt.emplace(Uri(base + "/keyboard"));
					hrt->hr.method = "PUT";
					hrt->hr.addHeader("Content-Type: application/json");
					hrt->hr.setPayload(payload->encode());
				}
			}
			else
			{
				hrt.emplace(Uri(base));
				hrt->hr.method = "DELETE";
			}
		}
		else if (hrt->tickUntilDone())
		{
			SOUP_IF_UNLIKELY (!hrt->result // Request failed?
				|| (deinit_requested && hrt->hr.method == "DELETE") // This was deinit request?
				)
			{
				setWorkDone();
			}
			hrt.reset();
		}
	}

	[[nodiscard]] static uint32_t razerEncodeColour(Rgb colour) noexcept
	{
		return (colour.b << 16) | (colour.g << 8) | colour.r;
	}

	bool kbRgbRazerChroma::controlsDevice(const hwHid& hid) const noexcept
	{
		return hid.vendor_id == 0x1532;
	}

	void kbRgbRazerChroma::deinit()
	{
		updateMaintainTask();
		if (task)
		{
			task->deinit_requested = true;
			sched.awaitCompletion();
		}
	}

	void kbRgbRazerChroma::setKey(Key key, Rgb colour)
	{
		for (const auto& [row, column] : mapKeyToPos(key))
		{
			const auto encoded = razerEncodeColour(colour);
			if (colours[row * 22 + column] != encoded)
			{
				colours[row * 22 + column] = encoded;
				commitColours();
			}
			else
			{
				ensureMaintainTask();
			}
		}
	}

	void kbRgbRazerChroma::setKeys(const Rgb(&colours)[NUM_KEYS])
	{
		bool changed = false;
		for (uint8_t row = 0; row != 6; ++row)
		{
			for (uint8_t column = 0; column != 22; ++column)
			{
				if (auto sk = mapPosToKey(row, column); sk != KEY_NONE)
				{
					const auto encoded = razerEncodeColour(colours[sk]);
					if (this->colours[row * 22 + column] != encoded)
					{
						this->colours[row * 22 + column] = encoded;
						changed = true;
					}
				}
			}
		}
		if (changed)
		{
			commitColours();
		}
		else
		{
			ensureMaintainTask();
		}
	}

	void kbRgbRazerChroma::setAllKeys(Rgb colour)
	{
		const auto encoded = razerEncodeColour(colour);

		bool changed = false;
		for (auto& clr : this->colours)
		{
			if (clr != encoded)
			{
				clr = encoded;
				changed = true;
			}
		}

		if (changed)
		{
			auto payload = soup::make_shared<JsonObject>();
			payload->add("effect", "CHROMA_STATIC");

			auto param = soup::make_unique<JsonObject>();
			param->add("color", soup::make_unique<JsonInt>(encoded));
			payload->add("param", std::move(param));

			getMaintainTask().payload = std::move(payload);
		}
		else
		{
			ensureMaintainTask();
		}
	}

	uint8_t kbRgbRazerChroma::getNumColumns() const noexcept
	{
		return 22;
	}

	Key kbRgbRazerChroma::mapPosToKey(uint8_t row, uint8_t column) const noexcept
	{
		if (row == 0)
		{
			switch (column)
			{
			case 0: return KEY_OEM_5;
			case 1: return KEY_ESCAPE;
			case 3: return KEY_F1;
			case 4: return KEY_F2;
			case 5: return KEY_F3;
			case 6: return KEY_F4;
			case 7: return KEY_F5;
			case 8: return KEY_F6;
			case 9: return KEY_F7;
			case 10: return KEY_F8;
			case 11: return KEY_F9;
			case 12: return KEY_F10;
			case 13: return KEY_F11;
			case 14: return KEY_F12;
			case 15: return KEY_PRINT_SCREEN;
			case 16: return KEY_PAUSE;
			case 17: return KEY_SCROLL_LOCK;
			case 18: return KEY_OEM_1;
			case 19: return KEY_OEM_2;
			case 20: return KEY_OEM_3;
			case 21: return KEY_OEM_4;
			}
		}
		else if (row == 1)
		{
			switch (column)
			{
			case 0: return KEY_OEM_6;
			case 1: return KEY_BACKQUOTE;
			case 2: return KEY_1;
			case 3: return KEY_2;
			case 4: return KEY_3;
			case 5: return KEY_4;
			case 6: return KEY_5;
			case 7: return KEY_6;
			case 8: return KEY_7;
			case 9: return KEY_8;
			case 10: return KEY_9;
			case 11: return KEY_0;
			case 12: return KEY_MINUS;
			case 13: return KEY_EQUALS;
			case 14: return KEY_BACKSPACE;
			case 15: return KEY_INSERT;
			case 16: return KEY_HOME;
			case 17: return KEY_PAGE_UP;
			case 18: return KEY_NUM_LOCK;
			case 19: return KEY_NUMPAD_DIVIDE;
			case 20: return KEY_NUMPAD_MULTIPLY;
			case 21: return KEY_NUMPAD_SUBTRACT;
			}
		}
		else if (row == 2)
		{
			switch (column)
			{
			case 0: return KEY_OEM_7;
			case 1: return KEY_TAB;
			case 2: return KEY_Q;
			case 3: return KEY_W;
			case 4: return KEY_E;
			case 5: return KEY_R;
			case 6: return KEY_T;
			case 7: return KEY_Y;
			case 8: return KEY_U;
			case 9: return KEY_I;
			case 10: return KEY_O;
			case 11: return KEY_P;
			case 12: return KEY_BRACKET_LEFT;
			case 13: return KEY_BRACKET_RIGHT;
			case 14: return KEY_BACKSLASH; // ANSI
			case 15: return KEY_DEL;
			case 16: return KEY_END;
			case 17: return KEY_PAGE_DOWN;
			case 18: return KEY_NUMPAD7;
			case 19: return KEY_NUMPAD8;
			case 20: return KEY_NUMPAD9;
			case 21: return KEY_NUMPAD_ADD;
			}
		}
		else if (row == 3)
		{
			switch (column)
			{
			case 0: return KEY_OEM_8;
			case 1: return KEY_CAPS_LOCK;
			case 2: return KEY_A;
			case 3: return KEY_S;
			case 4: return KEY_D;
			case 5: return KEY_F;
			case 6: return KEY_G;
			case 7: return KEY_H;
			case 8: return KEY_J;
			case 9: return KEY_K;
			case 10: return KEY_L;
			case 11: return KEY_SEMICOLON;
			case 12: return KEY_QUOTE;
			case 13: return KEY_BACKSLASH; // ISO
			case 14: return KEY_ENTER;
			case 18: return KEY_NUMPAD4;
			case 19: return KEY_NUMPAD5;
			case 20: return KEY_NUMPAD6;
			}
		}
		else if (row == 4)
		{
			switch (column)
			{
			case 0: return KEY_OEM_9;
			case 1: return KEY_LSHIFT;
			case 2: return KEY_INTL_BACKSLASH; // ISO
			case 3: return KEY_Z;
			case 4: return KEY_X;
			case 5: return KEY_C;
			case 6: return KEY_V;
			case 7: return KEY_B;
			case 8: return KEY_N;
			case 9: return KEY_M;
			case 10: return KEY_COMMA;
			case 11: return KEY_PERIOD;
			case 12: return KEY_SLASH;
			case 14: return KEY_RSHIFT;
			case 16: return KEY_ARROW_UP;
			case 18: return KEY_NUMPAD1;
			case 19: return KEY_NUMPAD2;
			case 20: return KEY_NUMPAD3;
			case 21: return KEY_NUMPAD_ENTER;
			}
		}
		else if (row == 5)
		{
			switch (column)
			{
			case 0: return KEY_OEM_10;
			case 1: return KEY_LCTRL;
			case 2: return KEY_LMETA;
			case 3: return KEY_LALT;
			case 7: return KEY_SPACE;
			case 11: return KEY_RALT;
			case 12: return KEY_FN;
			case 13: return KEY_CTX;
			case 14: return KEY_RCTRL;
			case 15: return KEY_ARROW_LEFT;
			case 16: return KEY_ARROW_DOWN;
			case 17: return KEY_ARROW_RIGHT;
			case 19: return KEY_NUMPAD0;
			case 20: return KEY_NUMPAD_DECIMAL;
			}
		}
		return KEY_NONE;

	}
}
#endif
