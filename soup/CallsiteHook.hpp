#pragma once

#include "HookBase.hpp"

NAMESPACE_SOUP
{
	// Hooks a typical callsite (E8 ? ? ? ?) using a code cave. See getCodeCavePattern.
	struct CallsiteHook : public HookBase
	{
		void* code_cave = nullptr;
		void* original = nullptr;
		
		void create();
		void destroy();

		void enable();
		void disable();
	};
}
