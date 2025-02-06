#include "ReplacementHook.hpp"

#include <climits> // INT_MIN, INT_MAX
#include <cstring> // memcpy

#include "Exception.hpp"
#include "ObfusString.hpp"
#include "pattern_macros.hpp"
#include "Pointer.hpp"

NAMESPACE_SOUP
{
	const uint8_t HookBase::jmp_trampoline[] = {
		0xE9, 0x00, 0x00, 0x00, 0x00, // jmp (4 bytes)
	};

	const uint8_t HookBase::call_trampoline[] = {
		0xE8, 0x00, 0x00, 0x00, 0x00, // call (4 bytes)
	};

	const uint8_t HookBase::longjump_trampoline_r10[] = {
		0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs r10, (8 bytes)
		0x41, 0xff, 0xe2, // jmp r10
	};

	const uint8_t HookBase::longjump_trampoline_noreg[] = {
		0xff, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp qword ptr [rip]
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // (8 bytes)
	};

	Pattern HookBase::getCodeCavePattern()
	{
		SIG_INST("CC CC CC CC CC CC CC CC CC CC CC CC CC");
		return sig_inst;
	}

	void* HookBase::getEffectiveTarget() const
	{
		return Pointer(target).followJumps().as<void*>();
	}

	void HookBase::writeJmpTrampoline(void* addr, void* target)
	{
		const auto rip_base = reinterpret_cast<uintptr_t>(addr) + 5;
		const ptrdiff_t rip_delta = reinterpret_cast<uintptr_t>(target) - rip_base;
		SOUP_IF_UNLIKELY (rip_delta < INT_MIN || rip_delta > INT_MAX)
		{
			SOUP_THROW(Exception(ObfusString("Unusable delta").str()));
		}

		uint8_t trampoline[sizeof(jmp_trampoline)];
		memcpy(trampoline, jmp_trampoline, sizeof(trampoline));
		*reinterpret_cast<int32_t*>(trampoline + 1) = static_cast<int32_t>(rip_delta);
		memcpy(addr, trampoline, sizeof(trampoline));
	}

	void HookBase::writeCallTrampoline(void* addr, void* target)
	{
		const auto rip_base = reinterpret_cast<uintptr_t>(addr) + 5;
		const ptrdiff_t rip_delta = reinterpret_cast<uintptr_t>(target) - rip_base;
		SOUP_IF_UNLIKELY (rip_delta < INT_MIN || rip_delta > INT_MAX)
		{
			SOUP_THROW(Exception(ObfusString("Unusable delta").str()));
		}

		uint8_t trampoline[sizeof(call_trampoline)];
		memcpy(trampoline, call_trampoline, sizeof(trampoline));
		*reinterpret_cast<int32_t*>(trampoline + 1) = static_cast<int32_t>(rip_delta);
		memcpy(addr, trampoline, sizeof(trampoline));
	}

	void HookBase::writeLongjumpTrampolineR10(void* addr, void* target) noexcept
	{
		uint8_t trampoline[sizeof(longjump_trampoline_r10)];
		memcpy(trampoline, longjump_trampoline_r10, sizeof(trampoline));
		*reinterpret_cast<void**>(trampoline + 2) = target;
		memcpy(addr, trampoline, sizeof(trampoline));
	}

	void HookBase::writeLongjumpTrampolineNoreg(void* addr, void* target) noexcept
	{
		uint8_t trampoline[sizeof(longjump_trampoline_noreg)];
		memcpy(trampoline, longjump_trampoline_noreg, sizeof(trampoline));
		*reinterpret_cast<void**>(trampoline + 6) = target;
		memcpy(addr, trampoline, sizeof(trampoline));
	}
}
