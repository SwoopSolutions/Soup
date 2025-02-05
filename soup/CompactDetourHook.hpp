#pragma once

#include "DetourHookBase.hpp"
#include "Pattern.hpp"

NAMESPACE_SOUP
{
	// Requires a unique code cave.
	struct CompactDetourHook : public DetourHookBase
	{
		void* code_cave = nullptr;

		[[nodiscard]] static Pattern getCodeCavePattern();

		[[nodiscard]] bool isCreated() const noexcept { return original != nullptr; }
		void create();
		void destroy();

		void enable();
		void disable();
	};
}
