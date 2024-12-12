#pragma once

#include "base.hpp"

#include <vector>

#include "hwHid.hpp"
#include "Key.hpp"

NAMESPACE_SOUP
{
	struct AnalogueKeyboard
	{
		std::string name;
		hwHid hid;
		bool has_ctx_key;
		bool disconnected = false;
		union
		{
			struct
			{
				uint8_t consecutive_empty_reports;
			} razer;
			struct
			{
				uint8_t am_version;
				uint8_t state;
				const uint8_t* layout;
				uint8_t buffer[NUM_KEYS];
			} keychron;
			struct
			{
				uint8_t buffer[NUM_KEYS];
			} nuphy;
			struct
			{
				uint8_t state;
				uint8_t layout_size;
				const Key* layout;
				uint8_t buffer[NUM_KEYS];
			} madlions;
		};

		AnalogueKeyboard() = default;
		AnalogueKeyboard(std::string&& name, hwHid&& hid, bool has_ctx_key);

		[[nodiscard]] static std::vector<AnalogueKeyboard> getAll(bool include_no_permission = false);

		class ActiveKey
		{
		public:
			Key sk;
			float fvalue;

			[[nodiscard]] Key getSoupKey() const noexcept
			{
				return sk;
			}

			[[nodiscard]] uint8_t getHidScancode() const noexcept;

			[[nodiscard]] uint16_t getPs2Scancode() const noexcept
			{
				return soup_key_to_ps2_scancode(sk);
			}

			[[nodiscard]] float getFValue() const noexcept
			{
				return fvalue;
			}

#if SOUP_WINDOWS
			// E.g., if the key to the right of T (KEY_Y) is pressed, will return 'Y'.
			[[nodiscard]] int getVk() const noexcept
			{
				return soup_key_to_virtual_key(getSoupKey());
			}

			// E.g. if the key to the right of T (KEY_Y) is pressed, may return 'Y' or 'Z' depending on user's layout.
			[[nodiscard]] int getVkTranslated() const noexcept
			{
				return soup_key_to_translated_virtual_key(getSoupKey());
			}
#endif
		};

		[[nodiscard]] bool havePermission() const noexcept
		{
			return hid.havePermission();
		}

		[[nodiscard]] bool isPoll() const noexcept;

		// This will block unless `isPoll()` or `hid.hasReport()` is true.
		[[nodiscard]] std::vector<ActiveKey> getActiveKeys();
	protected:
		[[nodiscard]] std::vector<ActiveKey> getActiveKeysMadlions();
	};
}
