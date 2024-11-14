#include "DetourHookBase.hpp"

#include <climits> // INT_MIN, INT_MAX
#include <cstring> // memcpy

#include "Exception.hpp"
#include "memGuard.hpp"
#include "ObfusString.hpp"
#include "Pointer.hpp"
#include "x64.hpp"

#define DH_DEBUG false

#if DH_DEBUG
#include <iostream>
#include "string.hpp"
#endif

NAMESPACE_SOUP
{
	uint8_t DetourHookBase::jmp_trampoline[] = {
		0xE9, 0x00, 0x00, 0x00, 0x00, // jmp (4 bytes)
	};

	uint8_t DetourHookBase::longjump_trampoline[] = {
		0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs r10, (8 bytes)
		0x41, 0xff, 0xe2, // jmp r10
	};

	void* DetourHookBase::getEffectiveTarget() const
	{
		return Pointer(target).followJumps().as<void*>();
	}

	void DetourHookBase::createOriginal(size_t trampoline_bytes)
	{
		const auto effective_target = getEffectiveTarget();
		auto op = (const uint8_t*)effective_target;
		size_t og_bytes = 0;
		do
		{
			auto op_start = op;
			auto ins = x64Disasm(op);
			if (!ins.isValid())
			{
				SOUP_THROW(Exception(ObfusString("Unsupported instruction").str()));
			}
			og_bytes += (op - op_start);
			for (const auto& opr : ins.operands)
			{
				if (opr.reg == soup::IP
					|| opr.reg == soup::DIS
					)
				{
					SOUP_THROW(Exception(ObfusString("Instruction interacts with instruction pointer").str()));
				}
			}
		} while (og_bytes < trampoline_bytes);

		original = malloc(og_bytes + sizeof(longjump_trampoline));
		memGuard::setAllowedAccess(original, og_bytes + sizeof(longjump_trampoline), memGuard::ACC_RWX);
		memcpy(original, effective_target, og_bytes);
		writeLongjumpTrampoline((uint8_t*)original + og_bytes, (uint8_t*)effective_target + og_bytes);

#if DH_DEBUG
		std::cout << "original proc: " << string::bin2hex(std::string((const char*)original, og_bytes + sizeof(longjump_trampoline))) << std::endl;
#endif
	}

	void DetourHookBase::destroyOriginal() noexcept
	{
		if (original != nullptr)
		{
			free(original);
			original = nullptr;
		}
	}

	void DetourHookBase::writeJmpTrampoline(void* addr, void* target)
	{
		const auto rip_base = reinterpret_cast<uintptr_t>(addr) + 5;
		const ptrdiff_t rip_delta = reinterpret_cast<uintptr_t>(target) - rip_base;
		SOUP_IF_UNLIKELY (rip_delta < INT_MIN || rip_delta > INT_MAX)
		{
			SOUP_THROW(Exception(ObfusString("Delta too big for jmp instruction").str()));
		}
		*(int32_t*)(jmp_trampoline + 1) = static_cast<int32_t>(rip_delta);
		memcpy(addr, jmp_trampoline, sizeof(jmp_trampoline));
	}

	void DetourHookBase::writeLongjumpTrampoline(void* addr, void* target) noexcept
	{
		*(void**)(longjump_trampoline + 2) = target;
		memcpy(addr, longjump_trampoline, sizeof(longjump_trampoline));
	}
}
