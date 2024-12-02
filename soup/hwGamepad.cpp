#include "hwGamepad.hpp"

#include <algorithm> // clamp
//#include <iostream>

#include "crc32.hpp"
#include "MemoryRefReader.hpp"
#include "os.hpp"
#include "Packet.hpp"
#include "time.hpp"

// Useful resources for DS4:
// - https://gitlab.com/CalcProgrammer1/OpenRGB/-/blob/master/Controllers/SonyGamepadController/SonyDS4Controller.cpp?ref_type=heads
// - http://eleccelerator.com/wiki/index.php?title=DualShock_4

// Useful resources for Switch:
// - https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md
// - https://wiki.handheldlegend.com/analog-stick-data

NAMESPACE_SOUP
{
	using Status = hwGamepad::Status;

	// DS4 may stay connected via BT despite being "powered off", so check if this one is actually live.
	[[nodiscard]] static bool isDs4StillAlive(hwHid& hid)
	{
		hid.discardStaleReports();
		for (int i = 0; i != 5; ++i)
		{
			if (hid.hasReport())
			{
				return true;
			}
			os::sleep(1);
		}
		return false;
	}

	std::vector<hwGamepad> hwGamepad::getAll()
	{
		std::vector<hwGamepad> res{};
		for (auto& hid : hwHid::getAll())
		{
			if (hid.usage_page == 1 && hid.usage == 5) // Gamepad
			{
				if (hid.vendor_id == 0x54c) // Sony
				{
					if (hid.product_id == 0xce6) // DualSense 5
					{
						res.emplace_back("DualSense 5", std::move(hid));
					}
					else if (hid.product_id == 0x5c4 || hid.product_id == 0x9cc) // DualShock 4 (rev 1 + 2)
					{
						if (isDs4StillAlive(hid))
						{
							res.emplace_back("DualShock 4", std::move(hid));
						}
					}
				}
				else if (hid.vendor_id == 0x57e // Nintendo
					&& hid.product_id == 0x2009 // Switch Pro Controller
					)
				{
					res.emplace_back("Nintendo Switch Pro Controller", std::move(hid)).switch_pro.has_calibration_data = false;
				}
				else if (hid.vendor_id == 0x18d1 // Google
					&& hid.product_id == 0x9400 // Stadia Controller
					)
				{
					Buffer buf;
					buf.push_back(0x05);
					bool is_bluetooth = !hid.sendReport(std::move(buf));
					res.emplace_back("Stadia Controller", std::move(hid)).stadia.is_bluetooth = is_bluetooth;
				}
#if false // Generic HID Gamepad support is pretty bad right now.
				else
				{
					res.emplace_back(nullptr, std::move(hid));
				}
#endif
			}
		}
		return res;
	}

	SOUP_PACKET(DsReport)
	{
		u8 left_stick_x;
		u8 left_stick_y;
		u8 right_stick_x;
		u8 right_stick_y;
		u8 buttons_1;
		u8 buttons_2;
		u8 buttons_3;
		u8 l2;
		u8 r2;
		u8 finger_1_data[4];
		u8 finger_2_data[4];

		// https://www.psdevwiki.com/ps4/DS4-USB#Data_Format
		bool readDs4(MemoryRefReader& s)
		{
			return s.u8(left_stick_x) // 1
				&& s.u8(left_stick_y) // 2
				&& s.u8(right_stick_x) // 3
				&& s.u8(right_stick_y) // 4
				&& s.u8(buttons_1) // 5
				&& s.u8(buttons_2) // 6
				&& s.u8(buttons_3) // 7
				&& s.u8(l2) // 8
				&& s.u8(r2) // 9
				//&& s.u16(timestamp) // 10-11
				//&& s.u8(battery) // 12
				//&& s.i16(gyro_x) // 13-14
				//&& s.i16(gyro_y) // 15-16
				//&& s.i16(gyro_z) // 17-18
				//&& s.i16(accel_x) // 19-20
				//&& s.i16(accel_y) // 21-22
				//&& s.i16(accel_z) // 23-24
				&& s.skip(35 - 10)
				&& s.u8(finger_1_data[0]) && s.u8(finger_1_data[1]) && s.u8(finger_1_data[2]) && s.u8(finger_1_data[3]) // 35-38
				&& s.u8(finger_2_data[0]) && s.u8(finger_2_data[1]) && s.u8(finger_2_data[2]) && s.u8(finger_2_data[3]) // 39-42
				;
		}

		// https://github.com/nondebug/dualsense
		bool readDs5Usb(MemoryRefReader& s)
		{
			return s.u8(left_stick_x) // 1
				&& s.u8(left_stick_y) // 2
				&& s.u8(right_stick_x) // 3
				&& s.u8(right_stick_y) // 4
				&& s.u8(l2) // 5
				&& s.u8(r2) // 6
				&& s.skip(1)
				&& s.u8(buttons_1) // 8
				&& s.u8(buttons_2) // 9
				&& s.u8(buttons_3) // 10
				&& s.skip(33 - 11)
				&& s.u8(finger_1_data[0]) && s.u8(finger_1_data[1]) && s.u8(finger_1_data[2]) && s.u8(finger_1_data[3])
				&& s.u8(finger_2_data[0]) && s.u8(finger_2_data[1]) && s.u8(finger_2_data[2]) && s.u8(finger_2_data[3])
				;
		}

		bool readDs5Bt01(MemoryRefReader& s)
		{
			return s.u8(left_stick_x) // 1
				&& s.u8(left_stick_y) // 2
				&& s.u8(right_stick_x) // 3
				&& s.u8(right_stick_y) // 4
				&& s.u8(buttons_1) // 5
				&& s.u8(buttons_2) // 6
				&& s.u8(buttons_3) // 7
				&& s.u8(l2) // 8
				&& s.u8(r2) // 9
				;
		}

		bool readDs5Bt31(MemoryRefReader& s)
		{
			return s.u8(left_stick_x) // 1
				&& s.u8(left_stick_y) // 2
				&& s.u8(right_stick_x) // 3
				&& s.u8(right_stick_y) // 4
				&& s.u8(l2) // 5
				&& s.u8(r2) // 6
				&& s.skip(1)
				&& s.u8(buttons_1) // 8
				&& s.u8(buttons_2) // 9
				&& s.u8(buttons_3) // 10
				&& s.skip(33 - 11)
				&& s.u8(finger_1_data[0]) && s.u8(finger_1_data[1]) && s.u8(finger_1_data[2]) && s.u8(finger_1_data[3])
				&& s.u8(finger_2_data[0]) && s.u8(finger_2_data[1]) && s.u8(finger_2_data[2]) && s.u8(finger_2_data[3])
				;
		}
	};

	SOUP_PACKET(StadiaReport)
	{
		u8 dpad;
		u8 buttons_1; // 01 = capture, 02 = assistant, 04 = LT, 08 = RT, 10 = meta, 20 = menu, 40 = options, 80 = RS
		u8 buttons_2; // 01 = LS, 02 = RB, 04 = LB, 08 = Y, 10 = X, 20 = B, 40 = A
		u8 left_stick_x;
		u8 left_stick_y;
		u8 right_stick_x;
		u8 right_stick_y;
		u8 left_trigger;
		u8 right_trigger;

		SOUP_PACKET_IO(s)
		{
			return s.skip(1) // report id (03)
				&& s.u8(dpad)
				&& s.u8(buttons_1)
				&& s.u8(buttons_2)
				&& s.u8(left_stick_x)
				&& s.u8(left_stick_y)
				&& s.u8(right_stick_x)
				&& s.u8(right_stick_y)
				&& s.u8(left_trigger)
				&& s.u8(right_trigger)
				;
		}
	};

	bool hwGamepad::hasStatusUpdate()
	{
		return hid.vendor_id == 0x54c // DS4 & DS5 send constant reports
			|| hid.hasReport()
			;
	}

	Status hwGamepad::receiveStatus()
	{
		Status status{};

		if (hid.vendor_id == 0x54c // Sony
			&& hid.product_id != 0xce6 // Not DS5
			&& !isDs4StillAlive(hid)
			)
		{
			disconnected = true;
			return status;
		}

		const Buffer& report_data = hid.receiveReport();

		// Ensure 'report_data' is the latest the device has to offer.
		// This works because the buffer ref returned is always the same.
		while (hid.hasReport())
		{
			SOUP_UNUSED(hid.receiveReport());
		}

		SOUP_IF_UNLIKELY (report_data.empty())
		{
			if (hid.vendor_id == 0x54c && hid.isBluetooth() && isDs4StillAlive(hid))
			{
				return receiveStatus();
			}
			disconnected = true;
		}
		else
		{
			update();

			//std::cout << string::bin2hex(report_data.toString(), true) << std::endl;

#if true
			if (hid.vendor_id == 0x54c) // Sony, Y Down
			{
				MemoryRefReader r(report_data);

				DsReport report;
				if (hid.product_id == 0xce6) // DS5
				{
					if (hid.isBluetooth())
					{
						if (report_data.at(0) == 0x31)
						{
							r.skip(2); // Report starts with 31 X1, where X counts up.
							report.readDs5Bt31(r);
						}
						else //if (report_data.at(0) == 0x01)
						{
							// This is a partial report without touchpad nor motion data.
							r.skip(1); // Report starts with 01
							report.readDs5Bt01(r);
							report.finger_1_data[0] = 0xff;
							report.finger_2_data[0] = 0xff;

							// Make the controller send a full report next time.
							// Data copied from "output report" textarea generated by this: https://github.com/nondebug/dualsense/blob/main/dualsense-explorer.html
							const uint8_t buf[] = { 0x31, 0x10, 0x10, 0xff, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x98, 0xb2, 0xea };
							hid.sendReport(buf, sizeof(buf));
						}
					}
					else
					{
						r.skip(1); // Report starts with 01
						report.readDs5Usb(r);
					}
				}
				else
				{
					if (report_data.at(0) == 0x11)
					{
						r.skip(3); // Bluetooth report starts with 11 C0 00
						ds4.is_bluetooth = true;
					}
					else if (report_data.at(0) == 0x05)
					{
						r.skip(1); // USB report starts with 05
						ds4.is_bluetooth = false;
					}
					else
					{
						// Sometimes report stats with 01, in which case we need to check the HID itself.
						r.skip(1);
						ds4.is_bluetooth = hid.isBluetooth();
					}
					report.readDs4(r);
				}

				status.left_stick_x = (static_cast<float>(report.left_stick_x) / 255.0f);
				status.left_stick_y = (static_cast<float>(report.left_stick_y) / 255.0f);
				status.right_stick_x = (static_cast<float>(report.right_stick_x) / 255.0f);
				status.right_stick_y = (static_cast<float>(report.right_stick_y) / 255.0f);
				status.left_trigger = (static_cast<float>(report.l2) / 255.0f);
				status.right_trigger = (static_cast<float>(report.r2) / 255.0f);
				status.setDpad(report.buttons_1 & 0xf);
				status.buttons[BTN_ACT_LEFT] = (report.buttons_1 & (1 << 4));
				status.buttons[BTN_ACT_DOWN] = (report.buttons_1 & (1 << 5));
				status.buttons[BTN_ACT_RIGHT] = (report.buttons_1 & (1 << 6));
				status.buttons[BTN_ACT_UP] = (report.buttons_1 & (1 << 7));
				status.buttons[BTN_LBUMPER] = (report.buttons_2 & (1 << 0));
				status.buttons[BTN_RBUMPER] = (report.buttons_2 & (1 << 1));
				status.buttons[BTN_LTRIGGER] = (report.buttons_2 & (1 << 2));
				status.buttons[BTN_RTRIGGER] = (report.buttons_2 & (1 << 3));
				status.buttons[BTN_SHARE] = (report.buttons_2 & (1 << 4));
				status.buttons[BTN_OPTIONS] = (report.buttons_2 & (1 << 5));
				status.buttons[BTN_LSTICK] = (report.buttons_2 & (1 << 6));
				status.buttons[BTN_RSTICK] = (report.buttons_2 & (1 << 7));
				status.buttons[BTN_META] = (report.buttons_3 & (1 << 0));
				status.buttons[BTN_TOUCHPAD] = (report.buttons_3 & (1 << 1));

				if (((report.finger_1_data[0] >> 7) & 1) == 0)
				{
					status.finger_coords[0].x = static_cast<float>((static_cast<unsigned int>(report.finger_1_data[2] & 0xf) << 8) | report.finger_1_data[1]);
					status.finger_coords[0].y = static_cast<float>((static_cast<unsigned int>(report.finger_1_data[3]) << 4) | (report.finger_1_data[2] >> 4));
					status.num_fingers_on_touchpad = 1;
				}
				if (((report.finger_2_data[0] >> 7) & 1) == 0)
				{
					status.finger_coords[status.num_fingers_on_touchpad].x = static_cast<float>((static_cast<unsigned int>(report.finger_2_data[2] & 0xf) << 8) | report.finger_2_data[1]);
					status.finger_coords[status.num_fingers_on_touchpad].y = static_cast<float>((static_cast<unsigned int>(report.finger_2_data[3]) << 4) | (report.finger_2_data[2] >> 4));
					status.num_fingers_on_touchpad++;
				}
			}
			else if (hid.vendor_id == 0x57e && hid.product_id == 0x2009) // Nintendo Switch Pro Controller, Y Up
			{
				SOUP_IF_UNLIKELY (!switch_pro.has_calibration_data)
				{
					calibrateSwitchProController();
				}

				uint16_t left_stick_x_raw, left_stick_y_raw, right_stick_x_raw, right_stick_y_raw;
				if (report_data[0] == 0x3f) // something with bluetooth + steam causes this report format to appear
				{
					left_stick_x_raw = (report_data[4] | (report_data[5] << 8)) / 0xF;
					left_stick_y_raw = (report_data[6] | (report_data[7] << 8)) / 0xF;
					right_stick_x_raw = (report_data[8] | (report_data[9] << 8)) / 0xF;
					right_stick_y_raw = (report_data[10] | (report_data[11] << 8)) / 0xF;

					// [1]: 01 = act down, 02 = act right, 04 = act left, 08 = act up, 10 = L1, 20 = R1, 40 = L2, 80 = R2
					// [2]: 01 = minus, 02 = plus, 04 = L3, 08 = R3, 10 = meta, 20 = capture
					// [3]: DPAD

					status.buttons[BTN_ACT_DOWN] = (report_data[1] & 0x01);
					status.buttons[BTN_ACT_RIGHT] = (report_data[1] & 0x02);
					status.buttons[BTN_ACT_LEFT] = (report_data[1] & 0x04);
					status.buttons[BTN_ACT_UP] = (report_data[1] & 0x08);
					status.buttons[BTN_LBUMPER] = (report_data[1] & 0x10);
					status.buttons[BTN_RBUMPER] = (report_data[1] & 0x20);
					status.buttons[BTN_LTRIGGER] = (report_data[1] & 0x40);
					status.buttons[BTN_RTRIGGER] = (report_data[1] & 0x80);
					status.buttons[BTN_MINUS] = (report_data[2] & 0x01);
					status.buttons[BTN_PLUS] = (report_data[2] & 0x02);
					status.buttons[BTN_LSTICK] = (report_data[2] & 0x04);
					status.buttons[BTN_RSTICK] = (report_data[2] & 0x08);
					status.buttons[BTN_META] = (report_data[2] & 0x10);
					status.buttons[BTN_SHARE] = (report_data[2] & 0x20);
					status.setDpad(report_data[3]);
				}
				else
				{
					left_stick_x_raw = report_data[6] | ((report_data[7] & 0xF) << 8);
					left_stick_y_raw = (report_data[7] >> 4) | (report_data[8] << 4);
					right_stick_x_raw = report_data[9] | ((report_data[10] & 0xF) << 8);
					right_stick_y_raw = (report_data[10] >> 4) | (report_data[11] << 4);

					// [3]: 01 = act left, 02 = act up, 04 = act down, 08 = act right, 40 = R1, 80 = R2
					// [4]: 01 = minus, 02 = plus, 04 = R3, 08 = L3, 10 = meta, 20 = capture
					// [5]: 01 = dpad down, 02 = dpad up, 04 = dpad right, 08 = dpad left, 40 = L1, 80 = L2

					status.buttons[BTN_ACT_LEFT] = (report_data[3] & 0x01);
					status.buttons[BTN_ACT_UP] = (report_data[3] & 0x02);
					status.buttons[BTN_ACT_DOWN] = (report_data[3] & 0x04);
					status.buttons[BTN_ACT_RIGHT] = (report_data[3] & 0x08);
					status.buttons[BTN_RBUMPER] = (report_data[3] & 0x40);
					status.buttons[BTN_RTRIGGER] = (report_data[3] & 0x80);
					status.buttons[BTN_MINUS] = (report_data[4] & 0x01);
					status.buttons[BTN_PLUS] = (report_data[4] & 0x02);
					status.buttons[BTN_RSTICK] = (report_data[4] & 0x04);
					status.buttons[BTN_LSTICK] = (report_data[4] & 0x08);
					status.buttons[BTN_META] = (report_data[4] & 0x10);
					status.buttons[BTN_SHARE] = (report_data[4] & 0x20);
					status.buttons[BTN_DPAD_DOWN] = (report_data[5] & 0x01);
					status.buttons[BTN_DPAD_UP] = (report_data[5] & 0x02);
					status.buttons[BTN_DPAD_RIGHT] = (report_data[5] & 0x04);
					status.buttons[BTN_DPAD_LEFT] = (report_data[5] & 0x08);
					status.buttons[BTN_LBUMPER] = (report_data[5] & 0x40);
					status.buttons[BTN_LTRIGGER] = (report_data[5] & 0x80);
				}

				status.left_stick_x = std::clamp(((float)(left_stick_x_raw - switch_pro.left_stick_x_min) / (float)switch_pro.left_stick_x_max_minus_min), 0.0f, 1.0f);
				status.left_stick_y = 1.0f - std::clamp(((float)(left_stick_y_raw - switch_pro.left_stick_y_min) / (float)switch_pro.left_stick_y_max_minus_min), 0.0f, 1.0f);
				status.right_stick_x = std::clamp(((float)(right_stick_x_raw - switch_pro.right_stick_x_min) / (float)switch_pro.right_stick_x_max_minus_min), 0.0f, 1.0f);
				status.right_stick_y = 1.0f - std::clamp(((float)(right_stick_y_raw - switch_pro.right_stick_y_min) / (float)switch_pro.right_stick_y_max_minus_min), 0.0f, 1.0f);
				status.left_trigger = status.buttons[BTN_LTRIGGER] ? 1.0f : 0.0f;
				status.right_trigger = status.buttons[BTN_RTRIGGER] ? 1.0f : 0.0f;
			}
			else if (hid.vendor_id == 0x18d1 && hid.product_id == 0x9400) // Stadia Controller, Y Down
			{
				MemoryRefReader r(report_data);

				StadiaReport report;
				report.read(r);

				// The sticks' min value seems to be 1
				if (report.left_stick_x != 0) { report.left_stick_x -= 1; }
				if (report.left_stick_y != 0) { report.left_stick_y -= 1; }
				if (report.right_stick_x != 0) { report.right_stick_x -= 1; }
				if (report.right_stick_y != 0) { report.right_stick_y -= 1; }
				status.left_stick_x = (static_cast<float>(report.left_stick_x) / 254.0f);
				status.left_stick_y = (static_cast<float>(report.left_stick_y) / 254.0f);
				status.right_stick_x = (static_cast<float>(report.right_stick_x) / 254.0f);
				status.right_stick_y = (static_cast<float>(report.right_stick_y) / 254.0f);

				status.left_trigger = (static_cast<float>(report.left_trigger) / 255.0f);
				status.right_trigger = (static_cast<float>(report.right_trigger) / 255.0f);

				status.setDpad(report.dpad);
				status.buttons[BTN_SHARE] = (report.buttons_1 & 0x01);
				status.buttons[BTN_ASSISTANT] = (report.buttons_1 & 0x02);
				status.buttons[BTN_LTRIGGER] = (report.buttons_1 & 0x04);
				status.buttons[BTN_RTRIGGER] = (report.buttons_1 & 0x08);
				status.buttons[BTN_META] = (report.buttons_1 & 0x10);
				status.buttons[BTN_MENU] = (report.buttons_1 & 0x20);
				status.buttons[BTN_OPTIONS] = (report.buttons_1 & 0x40);
				status.buttons[BTN_RSTICK] = (report.buttons_1 & 0x80);
				status.buttons[BTN_LSTICK] = (report.buttons_2 & 0x01);
				status.buttons[BTN_RBUMPER] = (report.buttons_2 & 0x02);
				status.buttons[BTN_LBUMPER] = (report.buttons_2 & 0x04);
				status.buttons[BTN_ACT_LEFT] = (report.buttons_2 & 0x08);
				status.buttons[BTN_ACT_UP] = (report.buttons_2 & 0x10);
				status.buttons[BTN_ACT_RIGHT] = (report.buttons_2 & 0x20);
				status.buttons[BTN_ACT_DOWN] = (report.buttons_2 & 0x40);
			}
#else
			if (false) { }
#endif
#if false
			else
			{
				//std::cout << string::bin2hex(report_data.toString()) << std::endl;
				const auto report_desc = hid.getReportDescriptor();
				const auto report = report_desc.parseInputReport(report_data.data(), report_data.size());

				/*std::cout << std::endl;
				for (const auto& sel : report.active_selectors)
				{
					std::cout << std::hex << sel.value << std::endl;
				}
				for (const auto& val : report.dynamic_values)
				{
					std::cout << std::hex << val.first.value << ": " << val.second << std::endl;
				}
				return status;*/

				status.left_stick_x = report.dynamic_values.get_or_default({ 1, 0x30 }, 0.5f);
				status.left_stick_y = report.dynamic_values.get_or_default({ 1, 0x31 }, 0.5f);

				if (auto lt = report.dynamic_values.find({ 2, 0xc5 }); lt != report.dynamic_values.end())
				{
					// Format used by Stadia Controller

					status.left_trigger = lt->second;
					status.right_trigger = report.dynamic_values.get_or_default({ 2, 0xc4 }, 0.0f);

					status.right_stick_x = report.dynamic_values.get_or_default({ 1, 0x32 }, 0.5f);
					status.right_stick_y = report.dynamic_values.get_or_default({ 1, 0x35 }, 0.5f);
				}
				else
				{
					// Format used by Wooting

					const float Z = report.dynamic_values.get_or_default({ 1, 0x32 }, 0.5f);
					if (fabsf(Z - 0.5f) > 0.00001f)
					{
						if (Z < 0.5f)
						{
							status.right_trigger = 1.0f - (Z * 2.0f);
						}
						else
						{
							status.left_trigger = (Z - 0.5f) * 2.0f;
						}
					}

					status.right_stick_x = report.dynamic_values.get_or_default({ 1, 0x33 }, 0.5f);
					status.right_stick_y = report.dynamic_values.get_or_default({ 1, 0x34 }, 0.5f);
				}

				status.setDpad(static_cast<uint8_t>(report.dynamic_values.get_or_default({ 1, 0x39 }, 1.0f) * 8.0f));

				// Captured from Stadia Controller. Mappings are slightly different on the DS4.
				status.buttons[BTN_ACT_DOWN] = report.active_selectors.count({ 9, 1 }) != 0;
				status.buttons[BTN_ACT_RIGHT] = report.active_selectors.count({ 9, 2 }) != 0;
				status.buttons[BTN_ACT_LEFT] = report.active_selectors.count({ 9, 4 }) != 0;
				status.buttons[BTN_ACT_UP] = report.active_selectors.count({ 9, 5 }) != 0;
				status.buttons[BTN_LBUMPER] = report.active_selectors.count({ 9, 7 }) != 0;
				status.buttons[BTN_RBUMPER] = report.active_selectors.count({ 9, 8 }) != 0;
				status.buttons[BTN_META] = report.active_selectors.count({ 9, 0xd }) != 0;
				status.buttons[BTN_LSTICK] = report.active_selectors.count({ 9, 0xe }) != 0;
				status.buttons[BTN_RSTICK] = report.active_selectors.count({ 9, 0xf }) != 0;
				status.buttons[BTN_RTRIGGER] = report.active_selectors.count({ 9, 0x13 }) != 0;
				status.buttons[BTN_LTRIGGER] = report.active_selectors.count({ 9, 0x14 }) != 0;
			}
#endif
		}

		return status;
	}

	bool hwGamepad::hasAnalogueTriggers() const noexcept
	{
		// Nintendo Switch Pro Controller does not have analogue triggers
		return hid.vendor_id != 0x57e;
	}

	bool hwGamepad::hasInvertedActionButtons() const noexcept
	{
		return hid.vendor_id == 0x57e; // Nintendo
	}

	bool hwGamepad::hasTouchpad() const noexcept
	{
		if (hid.vendor_id == 0x54c) // Sony
		{
			return true;
		}
		return false;
	}

	Vector2 hwGamepad::getTouchpadSize() const noexcept
	{
		if (hid.product_id == 0xce6)
		{
			return Vector2(1920.0f, 1080.0f); // DS5
		}
		return Vector2(1920.0f, 943.0f); // DS4
	}

	// Should maybe be called "has programmable light" because Stadia Controller does have a light, we just can't modify it...
	bool hwGamepad::hasLight() const noexcept
	{
		return hid.vendor_id == 0x54c && (hid.product_id == 0x5c4 || hid.product_id == 0x9cc); // DS4
		// DS5 probably also applies, but unsure about the output report format.
	}

	void hwGamepad::setLight(Rgb colour)
	{
		if (hasLight()) // no point in sending a no-op report
		{
			this->colour = colour;
			sendReport();
		}
	}

	bool hwGamepad::canRumble() const noexcept
	{
		if (hid.vendor_id == 0x54c) // Sony
		{
			if (hid.product_id == 0xce6) // DualSense 5
			{
				return false; // TODO: Look into this.
			}
			return true;
		}
		// TODO: Look into Nintendo Switch Pro Controller's rumble format.
		if (hid.vendor_id == 0x18d1 && hid.product_id == 0x9400) // Stadia Controller; rumble only works via USB
		{
			return !stadia.is_bluetooth;
		}
		return false;
	}

	void hwGamepad::rumbleWeak(uint8_t intensity, time_t ms)
	{
		this->weak_rumble_intensity = intensity;
		this->weak_rumble_until = time::millis() + ms;
		sendReport();
	}

	void hwGamepad::rumbleStrong(uint8_t intensity, time_t ms)
	{
		this->strong_rumble_intensity = intensity;
		this->strong_rumble_until = time::millis() + ms;
		sendReport();
	}

	void hwGamepad::update()
	{
		if (weak_rumble_intensity != 0
			&& time::millisUntil(weak_rumble_until) <= 0
			)
		{
			sendReport();
		}
		else if (strong_rumble_intensity != 0
			&& time::millisUntil(strong_rumble_until) <= 0
			)
		{
			sendReport();
		}
	}

	void hwGamepad::sendReport()
	{
		if (weak_rumble_intensity != 0
			&& time::millisUntil(weak_rumble_until) <= 0
			)
		{
			weak_rumble_intensity = 0;
		}
		if (strong_rumble_intensity != 0
			&& time::millisUntil(strong_rumble_until) <= 0
			)
		{
			strong_rumble_intensity = 0;
		}

		if (hid.vendor_id == 0x54c) // DS4
		{
			Buffer buf(11);
			buf.push_back(0x07);
			buf.push_back(0x00);
			buf.push_back(0x00);
			buf.push_back(weak_rumble_intensity);
			buf.push_back(strong_rumble_intensity);
			buf.push_back(colour.r);
			buf.push_back(colour.g);
			buf.push_back(colour.b);
			buf.push_back(0x00);
			buf.push_back(0x00);
			sendReportDs4(std::move(buf));
		}
		else if (hid.vendor_id == 0x18d1 && hid.product_id == 0x9400) // Stadia Controller
		{
			Buffer buf(5);
			buf.push_back(0x05);
			buf.push_back(strong_rumble_intensity);
			buf.push_back(strong_rumble_intensity);
			buf.push_back(weak_rumble_intensity);
			buf.push_back(weak_rumble_intensity);
			hid.sendReport(std::move(buf));
		}
	}

	void hwGamepad::sendReportDs4(Buffer&& buf) const
	{
		if (!ds4.is_bluetooth)
		{
			// Start data with 05
			buf.insert_front(1, 0x05);
		}
		else
		{
			// Start data with A2 11 C0 00
			buf.insert_front(1, 0x00);
			buf.insert_front(1, 0xC0);
			buf.insert_front(1, 0x11);
			buf.insert_front(1, 0xA2);

			buf.insert_back(75 - buf.size(), 0);
			uint32_t crc = crc32::hash(buf.data(), 75);
			buf.erase(0, 1); // hash needs to include the 0xA2, but we don't want to send that
			buf.push_back((0x000000FF & crc));
			buf.push_back((0x0000FF00 & crc) >> 8);
			buf.push_back((0x00FF0000 & crc) >> 16);
			buf.push_back((0xFF000000 & crc) >> 24);
		}
		hid.sendReport(std::move(buf));
	}

	void hwGamepad::calibrateSwitchProController()
	{
		Buffer buf(0x40);
		buf.push_back(0x01); // rumble + subcommand
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x10); // subcommand id: SPI flash read
		buf.push_back(0x3D); // address: 0x0000603D (Analogue stick calibration data)
		buf.push_back(0x60);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(9 + 9); // size
		hid.sendReport(std::move(buf));
		while (true)
		{
			const Buffer& report = hid.receiveReport();
			if (report.at(0) == 0x21)
			{
				switch_pro.has_calibration_data = true;

				const uint8_t* stick_cal = &report[20];

				uint16_t x_max_above_centre = ((stick_cal[1] << 8) & 0xF00) | stick_cal[0];
				uint16_t y_max_above_centre = (stick_cal[2] << 4) | (stick_cal[1] >> 4);
				uint16_t x_centre = ((stick_cal[4] << 8) & 0xF00) | stick_cal[3];
				uint16_t y_centre = (stick_cal[5] << 4) | (stick_cal[4] >> 4);
				uint16_t x_min_below_centre = ((stick_cal[7] << 8) & 0xF00) | stick_cal[6];
				uint16_t y_min_below_centre = (stick_cal[8] << 4) | (stick_cal[7] >> 4);

				switch_pro.left_stick_x_min = x_centre - x_min_below_centre;
				switch_pro.left_stick_x_max_minus_min = (x_centre + x_max_above_centre) - switch_pro.left_stick_x_min;
				switch_pro.left_stick_y_min = y_centre - y_min_below_centre;
				switch_pro.left_stick_y_max_minus_min = (y_centre + y_max_above_centre) - switch_pro.left_stick_y_min;

				stick_cal = &report[20 + 9];

				x_centre = ((stick_cal[1] << 8) & 0xF00) | stick_cal[0];
				y_centre = (stick_cal[2] << 4) | (stick_cal[1] >> 4);
				x_min_below_centre = ((stick_cal[4] << 8) & 0xF00) | stick_cal[3];
				y_min_below_centre = (stick_cal[5] << 4) | (stick_cal[4] >> 4);
				x_max_above_centre = ((stick_cal[7] << 8) & 0xF00) | stick_cal[6];
				y_max_above_centre = (stick_cal[8] << 4) | (stick_cal[7] >> 4);

				switch_pro.right_stick_x_min = x_centre - x_min_below_centre;
				switch_pro.right_stick_x_max_minus_min = (x_centre + x_max_above_centre) - switch_pro.right_stick_x_min;
				switch_pro.right_stick_y_min = y_centre - y_min_below_centre;
				switch_pro.right_stick_y_max_minus_min = (y_centre + y_max_above_centre) - switch_pro.right_stick_y_min;

				break;
			}
		}
	}

	void hwGamepad::Status::setDpad(uint8_t dpad)
	{
		switch (dpad)
		{
		case 0:
			buttons[BTN_DPAD_UP] = true;
			break;

		case 1:
			buttons[BTN_DPAD_UP] = true;
			buttons[BTN_DPAD_RIGHT] = true;
			break;

		case 2:
			buttons[BTN_DPAD_RIGHT] = true;
			break;

		case 3:
			buttons[BTN_DPAD_RIGHT] = true;
			buttons[BTN_DPAD_DOWN] = true;
			break;

		case 4:
			buttons[BTN_DPAD_DOWN] = true;
			break;

		case 5:
			buttons[BTN_DPAD_DOWN] = true;
			buttons[BTN_DPAD_LEFT] = true;
			break;

		case 6:
			buttons[BTN_DPAD_LEFT] = true;
			break;

		case 7:
			buttons[BTN_DPAD_LEFT] = true;
			buttons[BTN_DPAD_UP] = true;
			break;
		}
	}
}
