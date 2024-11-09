#pragma once

#include "hwHid.hpp"

#include "buttons.hpp"
#include "Rgb.hpp"
#include "Vector2.hpp"

NAMESPACE_SOUP
{
	// This is aimed at gamepads that report inputs via the HID standard, so the Xbox Controller will not be supported. (It's also uncomfortable af. Do yourself a favour and buy a proper controller. I can only recommend the DualShock 4.)
	// Currently, support for gamepads is highly specialised in hopes of providing a good API, but in future I might bother parsing the HID input report descriptor, so it can support generics half-decently.
	// With that out of the way, the list of currently supported gamepads is:
	// - Sony's DualSense 5
	// - Sony's DualShock 4
	// - Nintendo's Switch Pro Controller
	// - Google's Stadia Controller
	// They are supported via USB & BT, but BT tends to have less functionality, e.g. the Stadia Controller can't vibrate via BT.
	class hwGamepad
	{
	public:
		struct Status
		{
			float left_stick_x;
			float left_stick_y; // Y Down -- Top is 0.0f, bottom is 1.0f.
			float right_stick_x;
			float right_stick_y; // Y Down -- Top is 0.0f, bottom is 1.0f.
			float left_trigger;
			float right_trigger;
			alignas(float) Vector2 finger_coords[2]; // data is only initialised up to `num_fingers_on_touchpad`
			bool buttons[NUM_BUTTONS];
			uint8_t num_fingers_on_touchpad = 0;

			void setDpad(uint8_t dpad);
		};

		const char* name;
		hwHid hid;
		bool disconnected = false;
		union
		{
			struct
			{
				bool is_bluetooth;
			} ds4, stadia;
			struct
			{
				bool has_calibration_data;
				uint16_t left_stick_x_min;
				uint16_t left_stick_x_max_minus_min;
				uint16_t left_stick_y_min;
				uint16_t left_stick_y_max_minus_min;
				uint16_t right_stick_x_min;
				uint16_t right_stick_x_max_minus_min;
				uint16_t right_stick_y_min;
				uint16_t right_stick_y_max_minus_min;
			} switch_pro;
		};

		hwGamepad() = default;

		hwGamepad(const char* name, hwHid&& hid)
			: name(name), hid(std::move(hid))
		{
		}

	public:
		[[nodiscard]] static std::vector<hwGamepad> getAll();

		// Some gamepads pretty much always have something to report,
		// but the assumption should be that receiveStatus will block until the user causes a status update.
		[[nodiscard]] bool hasStatusUpdate();
		[[nodiscard]] Status receiveStatus();

		[[nodiscard]] bool hasAnalogueTriggers() const noexcept;
		[[nodiscard]] bool hasInvertedActionButtons() const noexcept;

		[[nodiscard]] bool hasTouchpad() const noexcept;
		[[nodiscard]] Vector2 getTouchpadSize() const noexcept;

	protected:
		Rgb colour = Rgb::BLACK;
	public:
		[[nodiscard]] bool hasLight() const noexcept;
		void setLight(Rgb colour);

		uint8_t weak_rumble_intensity = 0;
		time_t weak_rumble_until = 0;
		uint8_t strong_rumble_intensity = 0;
		time_t strong_rumble_until = 0;
		[[nodiscard]] bool canRumble() const noexcept;
		void rumbleWeak(uint8_t intensity, time_t ms);
		void rumbleStrong(uint8_t intensity, time_t ms);

		void update();
	private:
		void sendReport();
		void sendReportDs4(Buffer&& buf) const;

		void calibrateSwitchProController();
	};
}
