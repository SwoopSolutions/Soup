#pragma once

#include "DetourHookBase.hpp"

NAMESPACE_SOUP
{
	// Requires a unique code cave. See getCodeCavePattern.
	struct CompactDetourHook : public DetourHookBase
	{
		void* code_cave = nullptr;

		[[nodiscard]] bool isCreated() const noexcept { return original != nullptr; }
		void create();
		void destroy();

		void enable();
		void disable();
	};
}
