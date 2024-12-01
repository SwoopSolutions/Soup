#include "DetourHook.hpp"

#include <cstring> // memcpy

#include "memGuard.hpp"

NAMESPACE_SOUP
{
	void DetourHook::enable()
	{
		void* addr = getEffectiveTarget();
		memGuard::setAllowedAccess(addr, sizeof(longjump_trampoline_r10), memGuard::ACC_RWX);
		writeLongjumpTrampolineR10(addr, detour);
	}

	void DetourHook::disable()
	{
		memcpy(getEffectiveTarget(), original, sizeof(longjump_trampoline_r10));
	}
}
