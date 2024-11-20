#include "CpuInfo.hpp"
#if !SOUP_WASM

#if SOUP_X86
	#include "string.hpp"
#endif

NAMESPACE_SOUP
{
	std::string CpuInfo::toString() const SOUP_EXCAL
	{
#if SOUP_X86
		std::string str = "CPUID Support Level: ";
		str.append(string::hex(cpuid_max_eax));
		str.append("\nCPUID Extended Support Level: ");
		str.append(string::hex(cpuid_extended_max_eax & ~0x80000000));
		str.append("\nVendor: ");
		str.append(vendor_id.c_str());

		if (cpuid_max_eax >= 0x01)
		{
			str.append("\nStepping ID: ").append(std::to_string(stepping_id));
			str.append("\nModel: ").append(std::to_string(model));
			str.append("\nFamily: ").append(std::to_string(family));

			if (base_frequency || max_frequency || bus_frequency)
			{
				str.append("\nBase Frequency: ").append(std::to_string(base_frequency)).append(
					" MHz\n"
					"Max. Frequency: "
				).append(std::to_string(max_frequency)).append(
					" MHz\n"
					"Bus (Reference) Frequency: "
				).append(std::to_string(bus_frequency)).append(" MHz");
			}
		}

		str.append("\nSSE Support: ");
		if (supportsSSE4_2()) { str.append("SSE4.2"); }
		else if (supportsSSE4_1()) { str.append("SSE4.1"); }
		else if (supportsSSSE3()) { str.append("SSSE3"); }
		else if (supportsSSE3()) { str.append("SSE3"); }
		else if (supportsSSE2()) { str.append("SSE2"); }
		else if (supportsSSE()) { str.append("SSE"); }
		else { str.append("None"); }

		str.append("\nAVX Support: ");
		if (supportsAVX512F()) { str.append("AVX512F"); }
		else if (supportsAVX2()) { str.append("AVX2"); }
		else if (supportsAVX()) { str.append("AVX"); }
		else { str.append("None"); }

		std::string misc_features{};
		if (supportsPCLMULQDQ()) { string::listAppend(misc_features, "PCLMULQDQ"); }
		if (supportsAESNI()) { string::listAppend(misc_features, "AESNI"); }
		if (supportsRDRAND()) { string::listAppend(misc_features, "RDRAND"); }
		if (supportsRDSEED()) { string::listAppend(misc_features, "RDSEED"); }
		if (supportsSHA()) { string::listAppend(misc_features, "SHA"); }
		if (supportsSHA512()) { string::listAppend(misc_features, "SHA512"); }
		if (supportsXOP()) { string::listAppend(misc_features, "supportsXOP"); }
		str.append("\nOther Known Features: ").append(misc_features);

		return str;
#elif SOUP_ARM
		std::string str = "ARMv8 AES: ";
		str.append(armv8_aes ? "true" : "false");
		str.append("\nARMv8 SHA1: ").append(armv8_sha1 ? "true" : "false");
		str.append("\nARMv8 SHA2: ").append(armv8_sha2 ? "true" : "false");
		str.append("\nARMv8 CRC32: ").append(armv8_crc32 ? "true" : "false");
		return str;
#endif
	}
}

#endif
