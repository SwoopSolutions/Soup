#include "CompactDetourHook.hpp"

#include <cstring> // memset, memcpy

#include "memGuard.hpp"

NAMESPACE_SOUP
{
	void CompactDetourHook::create()
	{
		memGuard::setAllowedAccess(code_cave, sizeof(longjump_trampoline), memGuard::ACC_RWX);
		writeLongjumpTrampoline(code_cave, detour);

		createOriginal(sizeof(jmp_trampoline));
	}

	void CompactDetourHook::destroy()
	{
		memset(code_cave, 0xCC, sizeof(longjump_trampoline));

		destroyOriginal();
	}

	void CompactDetourHook::enable()
	{
		void* addr = getEffectiveTarget();
		memGuard::setAllowedAccess(addr, sizeof(jmp_trampoline), memGuard::ACC_RWX);
		writeJmpTrampoline(addr, code_cave);
	}

	void CompactDetourHook::disable()
	{
		memcpy(getEffectiveTarget(), original, sizeof(jmp_trampoline));
	}
}
