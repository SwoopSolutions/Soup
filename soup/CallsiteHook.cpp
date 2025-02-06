#include "CallsiteHook.hpp"

#include <cstring> // memset

#include "memGuard.hpp"
#include "Pointer.hpp"

NAMESPACE_SOUP
{
	void CallsiteHook::create()
	{
		original = Pointer(target).followJumps().add(1).rip().as<void*>();

		memGuard::setAllowedAccess(code_cave, sizeof(longjump_trampoline_r10), memGuard::ACC_RWX);
		writeLongjumpTrampolineR10(code_cave, detour);
	}

	void CallsiteHook::destroy()
	{
		memset(code_cave, 0xCC, sizeof(longjump_trampoline_r10));
	}

	void CallsiteHook::enable()
	{
		void* addr = getEffectiveTarget();
		memGuard::setAllowedAccess(addr, sizeof(call_trampoline), memGuard::ACC_RWX);
		writeCallTrampoline(addr, code_cave);
	}

	void CallsiteHook::disable()
	{
		writeCallTrampoline(getEffectiveTarget(), original);
	}
}
