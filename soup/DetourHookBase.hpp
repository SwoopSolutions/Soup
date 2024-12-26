#pragma once

#include "HookBase.hpp"

#include <cstddef> // size_t

#include "base.hpp"

NAMESPACE_SOUP
{
	struct DetourHookBase : public HookBase
	{
		void* original = nullptr;

		void createOriginal(size_t trampoline_bytes);
		void destroyOriginal() noexcept;
	};
}
