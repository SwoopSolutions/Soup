#pragma once

#include "DetourHookBase.hpp"

NAMESPACE_SOUP
{
	struct DetourHook : public DetourHookBase
	{
		[[nodiscard]] bool isCreated() const noexcept { return original != nullptr; }
		void create() { return createOriginal(sizeof(longjump_trampoline_r10)); }
		void destroy() noexcept { return destroyOriginal(); }

		void enable();
		void disable();
	};
}
