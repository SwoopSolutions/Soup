#pragma once

#include <cstdint>
#include <string>

#include "base.hpp"

NAMESPACE_SOUP
{
	enum x64Register : uint8_t
	{
		RA = 0,
		RC,
		RD,
		RB,
		SP,
		BP,
		SI,
		DI,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,
		IP,

		IMM,
		DIS,
		OFF,

		XMM0 = 0,
		XMM1,
		XMM2,
		XMM3,
		XMM4,
		XMM6,
		XMM7,
	};

	enum x64RegisterAccessType : uint8_t
	{
		ACCESS_64 = 64,
		ACCESS_32 = 32,
		ACCESS_16 = 16,
		ACCESS_8 = 8,
		ACCESS_8_H = 0,
	};

#pragma pack(push, 1)
	struct x64Operand
	{
		x64Register reg;
		union
		{
			struct // register
			{
				/* 0 */ x64RegisterAccessType access_type;
				/* 1 */ uint8_t deref_size;
				/* 2 */ int32_t deref_offset;
				/* 6 */
			};
			struct // immediate
			{
				/* 0 */ uint64_t val;
				/* 8 */
			};
			struct // displacement
			{
				/* 0 */ int64_t displacement;
				/* 8 */
			};
			struct // offset
			{
				/* 0 */ x64Register a;
				/* 1 */ x64Register b;
				/* 2 */
			} off;
		};

		void setReg(x64Register reg, x64RegisterAccessType access_type) noexcept
		{
			this->reg = reg;
			this->access_type = access_type;
			this->deref_size = 0;
			this->deref_offset = 0;
		}

		void setImm(uint64_t val) noexcept
		{
			this->reg = IMM;
			this->val = val;
		}

		void reset() noexcept
		{
			val = 0;
		}

		void decode(bool rex, uint8_t size, uint8_t reg, bool x) noexcept;
		void fromString(const char* str);

		[[nodiscard]] std::string toString(bool is_xmm = false) const;
	};
#pragma pack(pop)

	enum x64OperandEncoding : uint16_t
	{
		ZO = 0,

		O = 0b001,
		M = 0b010,
		R = 0b011,
		I = 0b100,
		D = 0b101,
		A = 0b111,

		OPERAND_MASK = 0b111,
		BITS_PER_OPERAND = 3,

		MR = M | (R << BITS_PER_OPERAND),
		RM = R | (M << BITS_PER_OPERAND),
		OI = O | (I << BITS_PER_OPERAND),
		MI = M | (I << BITS_PER_OPERAND),
		AI = A | (I << BITS_PER_OPERAND),

		RMI = R | (M << BITS_PER_OPERAND) | (I << (BITS_PER_OPERAND * 2)),
	};

	struct x64Operation
	{
		const char* const name;
		const uint32_t opcode;
		const x64OperandEncoding operand_encoding;
		const uint8_t operand_size = 0;
		const uint8_t distinguish = 8;
		const bool is_xmm = false;

		static constexpr uint8_t MAX_OPERANDS = 3;

		[[nodiscard]] uint32_t getUniqueId() const noexcept
		{
			return distinguish == 8
				? opcode
				: (opcode << 3) | distinguish
				;
		}

		[[nodiscard]] bool matches(const uint8_t* code) const noexcept
		{
			uint32_t b = *code;
			if (*code == 0xF3)
			{
				++code;
				b <<= 8;
				b |= *code;
			}
			if (*code == 0x0F)
			{
				++code;
				b <<= 8;
				b |= *code;
			}
			if (getOprEncoding(0) == O)
			{
				b &= ~0b111;
			}
			return opcode == b
				&& (distinguish == 8
					|| ((code[1] >> 3) & 0b111) == distinguish
					)
				;
		}

		[[nodiscard]] x64OperandEncoding getOprEncoding(uint8_t i) const noexcept
		{
			return (x64OperandEncoding)((operand_encoding >> (BITS_PER_OPERAND * i)) & OPERAND_MASK);
		}

		[[nodiscard]] uint8_t getNumOperands() const noexcept
		{
			uint8_t i = 0;
			while (i != MAX_OPERANDS)
			{
				if (getOprEncoding(i) == ZO)
				{
					break;
				}
				++i;
			}
			return i;
		}

		[[nodiscard]] uint8_t getNumModrmOperands() const noexcept
		{
			uint8_t count = 0;
			for (uint8_t i = 0; i != MAX_OPERANDS; ++i)
			{
				const auto enc = getOprEncoding(i);
				if (enc == M || enc == R)
				{
					++count;
				}
			}
			return count;
		}
	};

	inline static x64Operation operations[] = {
		{ "mov", 0x88, MR, 8 },
		{ "mov", 0x89, MR },
		{ "mov", 0x8A, RM, 8 },
		{ "mov", 0x8B, RM },
		{ "mov", 0xB0, OI, 8 },
		{ "mov", 0xB8, OI },
		{ "mov", 0xC6, MI, 8 },
		{ "mov", 0xC7, MI, 32 },
		{ "int3", 0xCC, ZO },
		{ "ret", 0xC3, ZO },
		{ "push", 0x50, O, 64 },
		{ "pop", 0x58, O, 64 },
		{ "push", 0xFF, M, 64, 6},
		{ "add", 0x81, MI, 32, 0 },
		{ "add", 0x83, MI, 8, 0 },
		{ "add", 0x03, RM },
		{ "or", 0x81, MI, 32, 1 },
		{ "or", 0x83, MI, 8, 1 },
		{ "or", 0x0A, RM },
		{ "and", 0x81, MI, 32, 4 },
		{ "and", 0x83, MI, 8, 4 },
		{ "and", 0x22, RM },
		{ "sub", 0x81, MI, 32, 5 },
		{ "sub", 0x83, MI, 8, 5 },
		{ "sub", 0x2B, RM, 32 },
		{ "sub", 0x29, MR },
		{ "cmp", 0x80, MI, 8, 7 },
		{ "cmp", 0x83, MI, 8, 7 },
		{ "cmp", 0x3C, AI, 8 },
		{ "cmp", 0x3D, AI, 32 },
		{ "cmp", 0x38, MR, 8 },
		{ "cmp", 0x39, MR },
		{ "cmp", 0x3A, RM, 8 },
		{ "cmp", 0x3B, RM },
		{ "lea", 0x8D, RM },
		{ "jz", 0x74, D, 8 },
		{ "jnz", 0x75, D, 8 },
		{ "call", 0xE8, D, 32 },
		{ "call", 0xFF, M, 32, 2 },
		{ "call", 0xFF, M, 32, 3 },
		{ "jmp", 0xEB, D, 8 },
		{ "jmp", 0xE9, D, 32 },
		{ "jmp", 0xFF, M, 32, 4 },
		{ "jmp", 0xFF, M, 32, 5 }, // TODO: Disassembly of "48 FF 60 50" should show "jmp qword ptr"
		{ "test", 0x84, MR, 8 },
		{ "test", 0x85, MR },
		{ "test", 0xF6, MI, 8, 0 },
		{ "test", 0xF7, MI, 32, 0 },
		{ "xor", 0x30, MR, 8 },
		{ "xor", 0x31, MR },
		{ "xor", 0x32, RM, 8 },
		{ "xor", 0x33, RM },
		{ "movzx", 0x0F'B6, RM, 8 },
		{ "movzx", 0x0F'B7, RM, 16 }, // TODO: Account for operands having different sizes
		{ "movsxd", 0x63, RM }, // TODO: Disassembly should show "dword ptr"
		{ "imul", 0x69, RMI, 32 },
		{ "nop", 0x90, ZO },
		{ "cpuid", 0x0F'A2, ZO },
		{ "div", 0xF7, M, 32, 6 },
		{ "movsx", 0x0F'BF, RM, 32 }, // TODO: Disassembly should show "word ptr"
		{ "inc", 0xFF, M, 0 },

		// XMM ops
		{ "movss", 0xF3'0F'10, RM, 0, 8, true },
		{ "movss", 0xF3'0F'11, MR, 0, 8, true },
		{ "movaps", 0x0F'28, RM, 0, 8, true },
		{ "movaps", 0x0F'29, MR, 0, 8, true },
		{ "xorps", 0x0F'57, RM, 0, 8, true },
	};

	struct x64Instruction
	{
		const x64Operation* operation = nullptr;
		x64Operand operands[x64Operation::MAX_OPERANDS];

		[[nodiscard]] bool isValid() const noexcept
		{
			return operation != nullptr;
		}

		void reset() noexcept;

		void fromString(const std::string& str);

		void setOperationFromOpcode(uint32_t opcode, uint8_t distinguish = 8);

		[[nodiscard]] std::string toString() const;
		[[nodiscard]] std::string toBytecode() const;
	};

	[[nodiscard]] extern std::string x64Asm(const std::string& code);
	extern x64Instruction x64Disasm(const uint8_t*& code);

	[[nodiscard]] extern uint8_t x64GetLength(const uint8_t* code);
	[[nodiscard]] extern const uint8_t* x64GetPrev(const uint8_t* code);
	[[nodiscard]] extern const uint8_t* x64GetNext(const uint8_t* code);
	[[nodiscard]] extern bool x64IsStartByte(const uint8_t* code);
}
