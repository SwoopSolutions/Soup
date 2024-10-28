#include "hwHid.hpp"

// Based on hidapi:
// - https://github.com/libusb/hidapi/blob/master/windows/hid.c
// - https://github.com/libusb/hidapi/blob/master/linux/hid.c

#if SOUP_WINDOWS
#include <cfgmgr32.h>
#include <hidsdi.h>

#pragma comment(lib, "CfgMgr32.lib")
#pragma comment(lib, "hid.lib")

#include "unicode.hpp"
#elif SOUP_LINUX
#include <sys/stat.h> // open
#include <fcntl.h> // O_RDWR etc
#include <unistd.h> // read
#include <poll.h>

#include <linux/hidraw.h>
#include <sys/ioctl.h>

#include "HidReportDescriptor.hpp"
#include "SharedLibrary.hpp"
#include "signal.hpp"
#include "string.hpp"

// https://github.com/systemd/systemd/blob/main/src/libudev/libudev.h

struct udev;
struct udev_enumerate;
struct udev_list_entry;
struct udev_device;

using udev_new_t = udev*(*)();
using udev_unref_t = udev*(*)(udev*);

using udev_enumerate_unref_t = udev_enumerate*(*)(udev_enumerate*);
using udev_enumerate_new_t = udev_enumerate*(*)(udev*);
using udev_enumerate_add_match_subsystem_t = int(*)(udev_enumerate*, const char*);
using udev_enumerate_scan_devices_t = int(*)(udev_enumerate*);
using udev_enumerate_get_list_entry_t = udev_list_entry*(*)(udev_enumerate*);

using udev_list_entry_get_next_t = udev_list_entry*(*)(udev_list_entry*);
using udev_list_entry_get_name_t = const char*(*)(udev_list_entry*);

using udev_device_unref_t = udev_device*(*)(udev_device*);
using udev_device_new_from_syspath_t = udev_device*(*)(udev*, const char*);
using udev_device_get_devnode_t = const char*(*)(udev_device*);
using udev_device_get_parent_with_subsystem_devtype_t = udev_device*(*)(udev_device*, const char*, const char*);
using udev_device_get_sysattr_value_t = const char*(*)(udev_device*, const char*);

#define udev_list_entry_foreach(list_entry, first_entry) for (list_entry = first_entry; list_entry; list_entry = udev_list_entry_get_next(list_entry))
#endif

NAMESPACE_SOUP
{
#if SOUP_LINUX
	[[nodiscard]] static bool parse_uevent_info(std::string uevent, unsigned short& vendor_id, unsigned short& product_id, std::string& product_name_utf8, std::string& serial_number_utf8)
	{
		char* saveptr = NULL;
		char* line;
		char* key;
		char* value;

		bool found_id = false;
		bool found_serial = false;
		bool found_name = false;

		line = strtok_r(uevent.data(), "\n", &saveptr);
		while (line != NULL)
		{
			/* line: "KEY=value" */
			key = line;
			value = strchr(line, '=');
			if (value)
			{
				*value = '\0';
				value++;

				if (strcmp(key, "HID_ID") == 0)
				{
					/**
					 *        type vendor   product
					 * HID_ID=0003:000005AC:00008242
					 **/
					unsigned bus_type;
					int ret = sscanf(value, "%x:%hx:%hx", &bus_type, &vendor_id, &product_id);
					if (ret == 3) {
						found_id = true;
					}
				}
				else if (strcmp(key, "HID_NAME") == 0)
				{
					product_name_utf8 = value;
					found_name = true;
				}
				else if (strcmp(key, "HID_UNIQ") == 0)
				{
					serial_number_utf8 = value;
					found_serial = true;
				}
			}
			line = strtok_r(NULL, "\n", &saveptr);
		}

		return (found_id && found_name && found_serial);
	}
#endif

	std::vector<hwHid> hwHid::getAll()
	{
		std::vector<hwHid> res{};
#if SOUP_WINDOWS
		GUID HIDGuid;
#if true
		HidD_GetHidGuid(&HIDGuid);
#else
		HIDGuid.Data1 = 0x4D1E55B2;
		HIDGuid.Data2 = 0xf16f;
		HIDGuid.Data3 = 0x11cf;
		HIDGuid.Data4[0] = 0x88;
		HIDGuid.Data4[1] = 0xcb;
		HIDGuid.Data4[2] = 0x00;
		HIDGuid.Data4[3] = 0x11;
		HIDGuid.Data4[4] = 0x11;
		HIDGuid.Data4[5] = 0x00;
		HIDGuid.Data4[6] = 0x00;
		HIDGuid.Data4[7] = 0x30;
#endif

		wchar_t* device_interface_list = nullptr;

		// Could also use SetupDiEnumDeviceInterfaces & SetupDiGetDeviceInterfaceDetailW to iterate devices,
		// but allocating it all in one go is preferable.
		CONFIGRET cr;
		do
		{
			DWORD len;
			cr = CM_Get_Device_Interface_List_SizeW(&len, &HIDGuid, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
			if (cr != CR_SUCCESS)
			{
				return {};
			}

			if (device_interface_list != nullptr)
			{
				free(device_interface_list);
			}
			device_interface_list = (wchar_t*)calloc(len, sizeof(wchar_t));
			SOUP_ASSERT(device_interface_list != nullptr);

			cr = CM_Get_Device_Interface_ListW(&HIDGuid, NULL, device_interface_list, len, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
			if (cr != CR_SUCCESS && cr != CR_BUFFER_SMALL)
			{
				return {};
			}
		} while (cr == CR_BUFFER_SMALL);

		for (const wchar_t* device_interface = device_interface_list; *device_interface; device_interface += wcslen(device_interface) + 1)
		{
			hwHid hid{};
			hid.path = unicode::utf16_to_utf8<std::wstring>(device_interface);

			hid.handle = CreateFileW(
				device_interface,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				NULL
			);
			if (!hid.handle.isValid())
			{
				continue;
			}

			HIDD_ATTRIBUTES attrib;
			attrib.Size = sizeof(HIDD_ATTRIBUTES);
			if (HidD_GetAttributes(hid.handle, &attrib))
			{
				hid.vendor_id = attrib.VendorID;
				hid.product_id = attrib.ProductID;

				PHIDP_PREPARSED_DATA pp_data = nullptr;
				if (HidD_GetPreparsedData(hid.handle, &pp_data))
				{
					HIDP_CAPS caps;
					if (HidP_GetCaps(pp_data, &caps) == HIDP_STATUS_SUCCESS)
					{
						hid.usage = caps.Usage;
						hid.usage_page = caps.UsagePage;
						hid.input_report_byte_length = caps.InputReportByteLength;
						hid.output_report_byte_length = caps.OutputReportByteLength;
						hid.feature_report_byte_length = caps.FeatureReportByteLength;

						// Example USB path: \\?\HID#VID_31E3&PID_1230&MI_02#8&39d6d828&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}
						// Example BT path: \\?\HID#{00001124-0000-1000-8000-00805f9b34fb}_VID&0002054c_PID&09cc#8&1e16befc&6&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}
						hid.is_bluetooth = (wcsstr(device_interface, L"HID#{") != nullptr);

						hid.read_buffer.reserve(caps.InputReportByteLength);

						res.emplace_back(std::move(hid));
					}
					HidD_FreePreparsedData(pp_data);
				}
			}
		}

		free(device_interface_list);
#elif SOUP_LINUX
		SharedLibrary libudev("libudev.so.1");
		SOUP_ASSERT(libudev.isLoaded(), "Failed to load libudev.so.1");

#define use_udev_func(name) const auto name = (name ## _t)libudev.getAddressMandatory(#name);

		use_udev_func(udev_new);
		use_udev_func(udev_unref);

		use_udev_func(udev_enumerate_unref);
		use_udev_func(udev_enumerate_new);
		use_udev_func(udev_enumerate_add_match_subsystem);
		use_udev_func(udev_enumerate_scan_devices);
		use_udev_func(udev_enumerate_get_list_entry);

		use_udev_func(udev_list_entry_get_next);
		use_udev_func(udev_list_entry_get_name);

		use_udev_func(udev_device_unref);
		use_udev_func(udev_device_new_from_syspath);
		use_udev_func(udev_device_get_devnode);
		use_udev_func(udev_device_get_parent_with_subsystem_devtype);
		use_udev_func(udev_device_get_sysattr_value);

		if (udev* udev = udev_new())
		{
			udev_enumerate* enumerate = udev_enumerate_new(udev);
			udev_enumerate_add_match_subsystem(enumerate, "hidraw");
			udev_enumerate_scan_devices(enumerate);

			udev_list_entry* devices;
			devices = udev_enumerate_get_list_entry(enumerate);

			udev_list_entry* entry;
			udev_list_entry_foreach(entry, devices)
			{
				const char* path = udev_list_entry_get_name(entry); // same as udev_device_get_syspath, /sys/devices/...

				hwHid hid{};
				hid.path = path;

				if (parse_uevent_info(string::fromFile(hid.path + "/device/uevent"), hid.vendor_id, hid.product_id, hid.product_name, hid.serial_number))
				{
					const auto rawdesc = string::fromFile(hid.path + "/device/report_descriptor");
					const auto report_desc = HidReportDescriptor::parse(rawdesc.data(), rawdesc.size());

					hid.usage_page = report_desc.usage_page;
					hid.usage = report_desc.usage;
					hid.input_report_byte_length = report_desc.input_report_byte_length;
					hid.output_report_byte_length = report_desc.output_report_byte_length;
					hid.feature_report_byte_length = report_desc.feature_report_byte_length;
					hid.report_ids = std::move(report_desc.report_ids);

					udev_device* device = udev_device_new_from_syspath(udev, path);
					const char* dev_path = udev_device_get_devnode(device); // /dev/hidraw0 etc

					hid.handle = ::open(dev_path, O_RDWR | O_CLOEXEC);

					// The product string we got from `parse_uevent_info` is a bit bad because it is actually vendor string + product string.
					if (auto usb_dev = udev_device_get_parent_with_subsystem_devtype(device, "usb", "usb_device"))
					{
						hid.manufacturer_name = udev_device_get_sysattr_value(usb_dev, "manufacturer");
						hid.product_name = udev_device_get_sysattr_value(usb_dev, "product");
					}

					// I don't quite trust input_report_byte_length yet, so ensuring a minimum of 1024 bytes.
					hid.read_buffer.reserve(hid.input_report_byte_length < 1024 ? 1024 : hid.input_report_byte_length);

					res.emplace_back(std::move(hid));

					udev_device_unref(device);
				}
			}

			udev_enumerate_unref(enumerate);
			udev_unref(udev);
		}
#endif
		return res;
	}

#if SOUP_WINDOWS
	std::string hwHid::getManufacturerName() const
	{
		std::string ret{};

		wchar_t buf[256];
		buf[0] = 0;
		if (HidD_GetManufacturerString(handle, buf, sizeof(buf)))
		{
			ret = unicode::utf16_to_utf8<std::wstring>(buf);
		}

		return ret;
	}

	std::string hwHid::getProductName() const
	{
		std::string ret{};

		wchar_t buf[256];
		buf[0] = 0;
		if (HidD_GetProductString(handle, buf, sizeof(buf)))
		{
			ret = unicode::utf16_to_utf8<std::wstring>(buf);
		}

		return ret;
	}

	std::string hwHid::getSerialNumber() const
	{
		std::string ret{};

		wchar_t buf[256];
		buf[0] = 0;
		if (HidD_GetSerialNumberString(handle, buf, sizeof(buf)))
		{
			ret = unicode::utf16_to_utf8<std::wstring>(buf);
		}

		return ret;
	}
#endif

	bool hwHid::isSamePhysicalDeviceAs(const hwHid& b) const
	{
		return vendor_id == b.vendor_id
			&& product_id == b.product_id
			&& getSerialNumber() == b.getSerialNumber()
			;
	}

	bool hwHid::isBluetooth() const noexcept
	{
#if SOUP_WINDOWS
		return is_bluetooth;
#else
		return false;
#endif
	}

	bool hwHid::hasReportId(uint8_t report_id) const noexcept
	{
		bool ret = false;
#if SOUP_WINDOWS
		PHIDP_PREPARSED_DATA pp_data = nullptr;
		if (HidD_GetPreparsedData(handle, &pp_data))
		{
			if (HidP_InitializeReportForID(HidP_Input, report_id, pp_data, (char*)read_buffer.data(), static_cast<ULONG>(read_buffer.capacity())) == HIDP_STATUS_SUCCESS)
			{
				ret = true;
			}
			HidD_FreePreparsedData(pp_data);
		}
#elif SOUP_LINUX
		ret = report_ids.count(report_id) != 0;
#endif
		return ret;
	}

	bool hwHid::hasReport() noexcept
	{
#if SOUP_WINDOWS
		if (pending_read == 0)
		{
			kickOffRead();
		}
		return pending_read == 1
			|| (pending_read == 2 && HasOverlappedIoCompleted(&read_overlapped))
			|| disconnected
			;
#elif SOUP_LINUX
		pollfd pfd;
		pfd.fd = handle;
		pfd.events = POLLIN;
		pfd.revents = 0;
		return poll(&pfd, 1, 0) != 0;
#else
		return true;
#endif
	}

#if SOUP_LINUX
	static bool setup_sig_handler = false;
	static thread_local bool killed_via_signal;
#endif

	// URB_INTERRUPT in
	const Buffer& hwHid::receiveReport() noexcept
	{
#if SOUP_WINDOWS
		SOUP_UNUSED(receiveReportWithReportId());
		if (!read_buffer.empty()
			&& read_buffer.at(0) == 0 // When a device uses report ids, `read` on Linux does prepend them, so to ensure we have the same result on both platforms, we only remove it on Windows when report ids are not being used.
			)
		{
			read_buffer.erase(0, 1);
		}
#elif SOUP_LINUX
		if (!setup_sig_handler)
		{
			signal::handle(SIGUSR1, [](int)
			{
				killed_via_signal = true;
			});
		}
		killed_via_signal = false;
		read_thrd = pthread_self();
		reading = true;
		int bytes_read = ::read(handle, read_buffer.data(), read_buffer.capacity());
		reading = false;
		read_buffer.resize(bytes_read < 0 ? 0 : bytes_read);
#endif
		return read_buffer;
	}

	const Buffer& hwHid::receiveReport(uint8_t& out_report_id) noexcept
	{
		out_report_id = 0;
#if SOUP_WINDOWS
		SOUP_UNUSED(receiveReportWithReportId());
		if (!read_buffer.empty())
		{
			out_report_id = read_buffer.at(0);
			read_buffer.erase(0, 1);
		}
#elif SOUP_LINUX
		SOUP_UNUSED(receiveReport());
		if (!report_ids.empty())
		{
			out_report_id = read_buffer.at(0);
			read_buffer.erase(0, 1);
		}
#endif
		return read_buffer;
	}

	// URB_INTERRUPT in
	const Buffer& hwHid::receiveReportWithReportId() noexcept
	{
#if SOUP_WINDOWS
		if (pending_read == 0)
		{
			kickOffRead();
		}
		if (pending_read != 0)
		{
			while (!GetOverlappedResult(handle, &read_overlapped, &bytes_read, FALSE)
				&& GetLastError() == ERROR_IO_INCOMPLETE
				)
			{
				Sleep(1);
			}
			pending_read = 0;
		}
		read_buffer.resize(bytes_read);
#elif SOUP_LINUX
		SOUP_UNUSED(receiveReport());
		if (report_ids.empty())
		{
			read_buffer.insert_front(1, 0);
		}
#endif
		return read_buffer;
	}

	// URB_INTERRUPT in
	const Buffer& hwHid::receiveReportWithoutReportId() noexcept
	{
#if SOUP_WINDOWS
		SOUP_UNUSED(receiveReportWithReportId());
		if (!read_buffer.empty())
		{
			read_buffer.erase(0, 1);
		}
#elif SOUP_LINUX
		SOUP_UNUSED(receiveReport());
		if (input_report_byte_length != 0
			&& !report_ids.empty()
			)
		{
			read_buffer.erase(0, 1);
		}
#endif
		return read_buffer;
	}

	void hwHid::discardStaleReports() noexcept
	{
#if SOUP_WINDOWS
		while (pending_read != 2 && !disconnected)
		{
			kickOffRead();
		}
#endif
	}

	void hwHid::cancelReceiveReport() noexcept
	{
#if SOUP_WINDOWS
		CancelIoEx(handle, &read_overlapped);
#elif SOUP_LINUX
		if (reading)
		{
			pthread_kill(read_thrd, SIGUSR1);
		}
#endif
	}

	// SET_REPORT response
	void hwHid::receiveFeatureReport(Buffer& buf) const
	{
#if SOUP_WINDOWS
		if (buf.size() < feature_report_byte_length)
		{
			buf.insert_back(feature_report_byte_length - buf.size(), '\0');
		}

		SOUP_ASSERT(HidD_GetFeature(handle, buf.data(), static_cast<ULONG>(buf.size())));
#elif SOUP_LINUX
		// TODO
#endif
	}

	bool hwHid::sendReport(Buffer&& buf) const noexcept
	{
#if SOUP_WINDOWS
		// The output report has to be at least as long as output_report_byte_length.
		// On Windows, we can make it work. On Linux, `write` will fail silently.
		if (buf.size() < output_report_byte_length)
		{
			buf.insert_back(output_report_byte_length - buf.size(), '\0');
		}
#endif
		return sendReport(buf.data(), buf.size());
	}

	// URB_INTERRUPT out
	bool hwHid::sendReport(const void* data, size_t size) const noexcept
	{
#if SOUP_WINDOWS
		OVERLAPPED overlapped{};
		DWORD bytesWritten;
		BOOL result = WriteFile(handle, data, static_cast<DWORD>(size), &bytesWritten, &overlapped);
		if (result == FALSE
			&& GetLastError() == ERROR_IO_PENDING
			)
		{
			result = GetOverlappedResult(handle, &overlapped, &bytesWritten, TRUE);
		}
		return result && bytesWritten == size;
#elif SOUP_LINUX
		return write(handle, data, size) == size;
#else
		return false;
#endif
	}

	// SET_REPORT request - bmRequestType = 0x21, bRequest = SET_REPORT (0x09), wValue = 0x0300 (ReportId = 0, ReportType = Feature (3))
	bool hwHid::sendFeatureReport(Buffer&& buf) const noexcept
	{
#if SOUP_WINDOWS
		// On Windows, the feature report has to be at least as long as feature_report_byte_length.
		if (buf.size() < feature_report_byte_length)
		{
			buf.insert_back(feature_report_byte_length - buf.size(), '\0');
		}

		return HidD_SetFeature(handle, buf.data(), static_cast<ULONG>(buf.size()));
#elif SOUP_LINUX
		return ioctl(handle, HIDIOCSFEATURE(buf.size()), buf.data()) == buf.size();
#else
		return false;
#endif
	}

#if SOUP_WINDOWS
	void hwHid::kickOffRead() noexcept
	{
		bytes_read = 0;
		if (disconnected)
		{
			return;
		}
		if (ReadFile(handle, read_buffer.data(), static_cast<DWORD>(read_buffer.capacity()), &bytes_read, &read_overlapped))
		{
			pending_read = 1;
		}
		else
		{
			SOUP_IF_LIKELY (GetLastError() == ERROR_IO_PENDING)
			{
				pending_read = 2;
			}
			else
			{
				// Device was likely disconnected (ERROR_DEVICE_NOT_CONNECTED), in which case subsequent calls to ReadFile will block.
				disconnected = true;
			}
		}
	}
#endif
}
