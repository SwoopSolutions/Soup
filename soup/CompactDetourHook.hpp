#pragma once

#include "DetourHookBase.hpp"

NAMESPACE_SOUP
{
	// Requires a code cave of at least 13 bytes, e.g. Pattern("CC CC CC CC CC CC CC CC CC CC CC CC CC")
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
