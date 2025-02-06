#pragma once

#include <cstdint> // uint8_t

#include "base.hpp"

#include "Pattern.hpp"

NAMESPACE_SOUP
{
	struct HookBase
	{
		void* detour = nullptr;
		void* target = nullptr;

		static const uint8_t jmp_trampoline[5];
		static const uint8_t call_trampoline[5];
		static const uint8_t longjump_trampoline_r10[13];
		static const uint8_t longjump_trampoline_noreg[14];

		[[nodiscard]] static Pattern getCodeCavePattern();

		[[nodiscard]] void* getEffectiveTarget() const;

		static void writeJmpTrampoline(void* addr, void* target);
		static void writeCallTrampoline(void* addr, void* target);
		static void writeLongjumpTrampolineR10(void* addr, void* target) noexcept;
		static void writeLongjumpTrampolineNoreg(void* addr, void* target) noexcept;
	};
}
