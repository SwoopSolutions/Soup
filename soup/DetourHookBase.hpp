#pragma once

#include <cstddef> // size_t
#include <cstdint> // uint8_t

#include "base.hpp"

NAMESPACE_SOUP
{
	struct DetourHookBase
	{
		void* detour = nullptr;
		void* target = nullptr;
		void* original = nullptr;

		static const uint8_t jmp_trampoline[5];
		static const uint8_t longjump_trampoline_r10[13];
		static const uint8_t longjump_trampoline_noreg[14];

		[[nodiscard]] void* getEffectiveTarget() const;
		void createOriginal(size_t trampoline_bytes);
		void destroyOriginal() noexcept;

		static void writeJmpTrampoline(void* addr, void* target);
		static void writeLongjumpTrampolineR10(void* addr, void* target) noexcept;
		static void writeLongjumpTrampolineNoreg(void* addr, void* target) noexcept;
	};
}
