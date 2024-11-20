#include "Hwid.hpp"
#if SOUP_WINDOWS && SOUP_X86

#include <set>
#include <string>

#include <windows.h>
#pragma comment(lib, "user32.lib") // EnumDisplayDevicesA

#include "bitutil.hpp"
#include "CpuInfo.hpp"
#include "joaat.hpp"

NAMESPACE_SOUP
{
	Hwid Hwid::get()
	{
		Hwid hwid;

		{
			const CpuInfo& ci = CpuInfo::get();
			hwid.cpu_hash = joaat::hashRange(ci.vendor_id.data(), 16);
			hwid.cpu_hash = joaat::hashRange((const char*)&ci.feature_flags_ecx, 4, hwid.cpu_hash);
			hwid.cpu_hash = joaat::hashRange((const char*)&ci.feature_flags_edx, 4, hwid.cpu_hash);
			hwid.cpu_hash = joaat::hashRange((const char*)&ci.stepping_id, 1, hwid.cpu_hash);
			hwid.cpu_hash = joaat::hashRange((const char*)&ci.model, 1, hwid.cpu_hash);
			hwid.cpu_hash = joaat::hashRange((const char*)&ci.family, 1, hwid.cpu_hash);
			hwid.cpu_hash = joaat::hashRange((const char*)&ci.extended_features_0_ebx, 4, hwid.cpu_hash);
			hwid.cpu_hash = joaat::hashRange((const char*)&ci.extended_flags_1_ecx, 4, hwid.cpu_hash);
		}

		{
			std::set<std::string> gpu_set{};
			for (int i = 0;; i++)
			{
				DISPLAY_DEVICEA dd;
				dd.cb = sizeof(dd);
				if (!EnumDisplayDevicesA(NULL, i, &dd, EDD_GET_DEVICE_INTERFACE_NAME))
				{
					break;
				}
				gpu_set.emplace(dd.DeviceString);
			}
			uint32_t gpu_hash = 0;
			for (const auto& gpu : gpu_set)
			{
				gpu_hash = joaat::hash(gpu, gpu_hash);
			}
			hwid.gpu_hash = gpu_hash & 0xff'ff'ff;
		}

		// https://stackoverflow.com/a/2513561
		{
			MEMORYSTATUSEX status;
			status.dwLength = sizeof(status);
			GlobalMemoryStatusEx(&status);
			hwid.ram_mag = bitutil::getMostSignificantSetBit(static_cast<unsigned int>(status.ullTotalPhys / 1'000'000'000));
		}

		return hwid;
	}
}
#endif
