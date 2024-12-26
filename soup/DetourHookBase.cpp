#include "DetourHookBase.hpp"

#include "alloc.hpp"
#include "Exception.hpp"
#include "memGuard.hpp"
#include "ObfusString.hpp"
#include "x64.hpp"

#define DH_DEBUG false

#if DH_DEBUG
#include <iostream>
#include "string.hpp"
#endif

NAMESPACE_SOUP
{
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

		original = soup::malloc(og_bytes + sizeof(longjump_trampoline_noreg));
		memGuard::setAllowedAccess(original, og_bytes + sizeof(longjump_trampoline_noreg), memGuard::ACC_RWX);
		memcpy(original, effective_target, og_bytes);
		writeLongjumpTrampolineNoreg((uint8_t*)original + og_bytes, (uint8_t*)effective_target + og_bytes);

#if DH_DEBUG
		std::cout << "original proc: " << string::bin2hex(std::string((const char*)original, og_bytes + sizeof(longjump_trampoline_noreg))) << std::endl;
#endif
	}

	void DetourHookBase::destroyOriginal() noexcept
	{
		if (original != nullptr)
		{
			soup::free(original);
			original = nullptr;
		}
	}
}
