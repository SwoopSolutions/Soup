#include "DetourHook.hpp"

#include <cstring> // memcpy

#include "memGuard.hpp"

NAMESPACE_SOUP
{
	void DetourHook::enable()
	{
		void* addr = getEffectiveTarget();
		memGuard::setAllowedAccess(addr, sizeof(longjump_trampoline), memGuard::ACC_RWX);
		writeLongjumpTrampoline(addr, detour);
	}

	void DetourHook::disable()
	{
		memcpy(getEffectiveTarget(), original, sizeof(longjump_trampoline));
	}
}
