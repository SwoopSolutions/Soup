#include "x64.hpp"

#include "Endian.hpp"
#include "Exception.hpp"
#include "macros.hpp"
#include "string.hpp"
#include "StringBuilder.hpp"

NAMESPACE_SOUP
{
	static const char* reg_names[] = {
		"a",
		"c",
		"d",
		"b",
		"sp",
		"bp",
		"si",
		"di",
		"8",
		"9",
		"10",
		"11",
		"12",
		"13",
		"14",
		"15",
		"ip",
	};

	void x64Operand::decode(bool rex, uint8_t size, uint8_t reg, bool x) noexcept
	{
		reg |= (x << 3);

		this->reg = (x64Register)reg;
		access_type = (x64RegisterAccessType)size;

		if (reg >= SP && reg <= DI
			&& !rex
			&& size == 8
			)
		{
			this->reg = (x64Register)(this->reg - 4);
			access_type = ACCESS_8_H;
		}
	}

	std::string x64Operand::toString(bool is_xmm) const
	{
		if (reg == IMM)
		{
			return std::to_string(val);
		}
		if (reg == DIS)
		{
			std::string str;
			if (displacement < 0)
			{
				str = "[rip-0x";
				str.append(string::hex(displacement * -1));
			}
			else
			{
				str = "[rip+0x";
				str.append(string::hex(displacement));
			}
			str.push_back(']');
			return str;
		}
		if (reg == OFF)
		{
			std::string str;
			str.push_back('[');
			str.push_back('r');
			str.append(reg_names[off.a]);
			if (off.a < SP)
			{
				str.push_back('x');
			}
			str.push_back('+');
			str.push_back('r');
			str.append(reg_names[off.b]);
			if (off.b < SP)
			{
				str.push_back('x');
			}
			str.push_back(']');
			return str;
		}

		std::string name{};
		if (is_xmm && deref_size == 0)
		{
			name = "xmm";
			name.append(std::to_string((int)reg));
		}
		else if (reg < R8)
		{
			name = reg_names[reg];
			if (access_type == ACCESS_8_H)
			{
				name.push_back('h');
			}
			else if (access_type == ACCESS_8)
			{
				name.push_back('l');
			}
			else // >= 16-bit
			{
				if (reg < SP)
				{
					name.push_back('x');
				}
				if (access_type == ACCESS_32)
				{
					name.insert(0, 1, 'e');
				}
				else if (access_type == ACCESS_64)
				{
					name.insert(0, 1, 'r');
				}
			}
		}
		else // >= R8
		{
			name.push_back('r');
			name.append(reg_names[reg]);
			if (access_type == ACCESS_32)
			{
				name.push_back('d');
			}
			else if (access_type == ACCESS_16)
			{
				name.push_back('w');
			}
			else if (access_type == ACCESS_8)
			{
				name.push_back('l');
			}
		}
		if (deref_size != 0)
		{
			name.insert(0, 1, '[');
			if (deref_size == 8)
			{
				name.insert(0, "byte ptr ");
			}
			else if (deref_size == 16)
			{
				name.insert(0, "word ptr ");
			}
			else if (deref_size == 32)
			{
				name.insert(0, "dword ptr ");
			}
			else if (deref_size == 64)
			{
				name.insert(0, "qword ptr ");
			}
			if (deref_offset != 0)
			{
				if (deref_offset < 0)
				{
					name.append("-0x");
					name.append(string::hex(deref_offset * -1));
				}
				else
				{
					name.append("+0x");
					name.append(string::hex(deref_offset));
				}
			}
			name.push_back(']');
		}
		return name;
	}

	static void decodeAccessType(x64RegisterAccessType& typ, const char*& str)
	{
		++str; // skip last char of register name
		
		if (typ == ACCESS_64 || typ == ACCESS_32)
		{
			++str; // skip 'x'
		}
		else if (*str == 'x')
		{
			typ = ACCESS_16;
			++str;
		}
		else if (*str == 'l')
		{
			typ = ACCESS_8;
			++str;
		}
		else if (*str == 'h')
		{
			typ = ACCESS_8_H;
			++str;
		}
	}

	void x64Operand::fromString(const char* str)
	{
		access_type = ACCESS_16;

		while (*str == ' ')
		{
			++str;
		}

		if (*str == '[')
		{
			deref_size = 32;
			++str;
		}

		if (*str == 'r')
		{
			access_type = ACCESS_64;
			++str;
		}
		else if (*str == 'e')
		{
			access_type = ACCESS_32;
			++str;
		}

		if (*str == 'a')
		{
			reg = RA;
			decodeAccessType(access_type, str);
		}
		else if (*str == 'c')
		{
			reg = RC;
			decodeAccessType(access_type, str);
		}
		else if (*str == 'b')
		{
			reg = RB;
			decodeAccessType(access_type, str);
		}
		else if (*str == 'd')
		{
			if (str[1] == 'i')
			{
				reg = DI;
				str += 2;
			}
			else
			{
				reg = RD;
				decodeAccessType(access_type, str);
			}
		}
		else if (*str == 's' && *++str == 'i')
		{
			reg = SI;
			++str;
		}

		if (*str == '+')
		{
			deref_offset = 0;
			while (string::isNumberChar(*++str))
			{
				deref_offset *= 10;
				deref_offset += ((*str) - '0');
			}
		}

		if (*str == ']')
		{
			SOUP_ASSERT(deref_size == 32); // We had '['?
			++str;
		}

		SOUP_ASSERT(*str == '\0');
	}

	void x64Instruction::reset() noexcept
	{
		operation = nullptr;
		for (auto& opr : operands)
		{
			opr.reset();
		}
	}

	void x64Instruction::fromString(const std::string& str)
	{
		reset();

		StringBuilder op;
		auto it = str.begin();
		op.beginCopy(str, it);
		uint8_t opr_i = 0;
		for (; it != str.end(); ++it)
		{
			if (*it == ' ')
			{
				op.endCopy(str, it);
				++it;
				StringBuilder opr;
				opr.beginCopy(str, it);
				for (; it != str.end(); ++it)
				{
					if (*it == ',')
					{
						opr.endCopy(str, it);
						operands[opr_i++].fromString(opr.c_str());
						opr.clear();
						if (++it == str.end())
						{
							opr.beginCopy(str, it);
							break;
						}
						opr.beginCopy(str, it);
					}
				}
				opr.endCopy(str, it);
				if (!opr.empty())
				{
					operands[opr_i++].fromString(opr.c_str());
				}
				break;
			}
		}
		if (op.empty())
		{
			op.endCopy(str, it);
		}

		for (const auto& opinfo : operations)
		{
			if (opinfo.name == op
				&& opinfo.getNumOperands() == opr_i
				&& opinfo.operand_size != 8
				)
			{
				operation = &opinfo;
				break;
			}
		}
		if (operation == nullptr)
		{
			std::string err = "Unknown x64 instruction: ";
			err.append(op);
			SOUP_THROW(Exception(std::move(err)));
		}
	}

	void x64Instruction::setOperationFromOpcode(uint32_t opcode, uint8_t distinguish)
	{
		operation = nullptr;
		for (const auto& opinfo : operations)
		{
			if (opinfo.opcode == opcode
				&& opinfo.distinguish == distinguish
				)
			{
				operation = &opinfo;
				break;
			}
		}
	}

	std::string x64Instruction::toString() const
	{
		// This could be a bit cleverer and show e.g. "imul rax, 100" instead of "imul rax, rax, 100"
		std::string res = operation->name;
		if (operation->getOprEncoding(0) != ZO)
		{
			res.push_back(' ');
			res.append(operands[0].toString(operation->is_xmm));
			if (operation->getOprEncoding(1) != ZO)
			{
				res.append(", ");
				res.append(operands[1].toString(operation->is_xmm));
				if (operation->getOprEncoding(2) != ZO)
				{
					res.append(", ");
					res.append(operands[2].toString(operation->is_xmm));
				}
			}
		}
		return res;
	}

	std::string x64Instruction::toBytecode() const
	{
		std::string res{};

		if (operands[0].access_type == ACCESS_16)
		{
			res.push_back('\x66');
		}

		uint8_t rex = 0;
		if (operands[0].access_type == ACCESS_64
			&& strcmp(operation->name, "jmp") != 0
			)
		{
			rex |= (1 << 3);
		}
		if (operation->getOprEncoding(0) == R && (operands[0].reg & 0x8))
		{
			rex |= (1 << 2);
		}
		if (operation->getOprEncoding(1) == M && operands[1].reg == OFF && (operands[1].off.b & 0x8))
		{
			rex |= (1 << 1);
		}
		if ((operation->getOprEncoding(0) == O && (operands[0].reg & 0x8))
			|| (operation->getOprEncoding(0) == M && (operands[0].reg & 0x8))
			|| (operation->getOprEncoding(1) == M
				&& (
					(operands[1].reg & 0x8)
					|| (operands[1].reg == OFF && (operands[1].off.a & 0x8))
				))
			)
		{
			rex |= (1 << 0);
		}
		if (rex != 0)
		{
			rex |= 0x40;
			res.push_back(rex);
		}

		if ((operation->opcode >> 8) & 0xFF)
		{
			res.push_back((operation->opcode >> 8) & 0xFF);
		}
		uint8_t opcode = (operation->opcode & 0xFF);
		if (operation->getOprEncoding(0) == O)
		{
			opcode |= (operands[0].reg & 0b111);
		}
		res.push_back(opcode);

		if (operation->getNumModrmOperands() != 0)
		{
			uint8_t modrm = ((operation->distinguish & 0b111) << 3);
			if (operands[1].reg == OFF)
			{
				modrm |= ((operands[0].reg & 0b111) << 3); // destination
				modrm |= 4;
			}
			else
			{
				if (operands[0].deref_size == 0)
				{
					modrm |= (0b11 << 6); // direct
				}
				else if (operands[0].deref_offset != 0)
				{
					modrm |= (0b01 << 6);
				}
				if (operation->getNumOperands() != 1)
				{
					modrm |= ((operands[1].reg & 0b111) << 3); // reg
				}
				modrm |= (operands[0].reg & 0b111); // r/m
			}
			res.push_back(modrm);
			if (operands[1].reg == OFF)
			{
				uint8_t sib = 0;
				sib |= (operands[1].off.a & 0b111);
				sib |= ((operands[1].off.b & 0b111) << 3);
				res.push_back(sib);
			}
			if (operands[0].deref_size != 0
				&& operands[0].deref_offset != 0
				)
			{
				res.push_back(operands[0].deref_offset);
			}
		}

		if (operation->getOprEncoding(1) == I)
		{
			static_assert(ENDIAN_NATIVE == ENDIAN_LITTLE);
			if (operands[0].access_type == ACCESS_64)
			{
				res.append((const char*)&operands[1].val, 8);
			}
			else
			{
				res.append((const char*)&operands[1].val, 4);
			}
		}

		return res;
	}

	// https://www.felixcloutier.com/x86/index.html

	template <typename T = uint64_t>
	static void getImmediate(T& val, const uint8_t*& code, const uint8_t imm_bytes)
	{
		++code;
		val = 0;
		for (uint8_t i = imm_bytes; i-- != 0; )
		{
			val <<= 8;
			val |= code[i];
		}
		code += (imm_bytes - 1);
	}

	std::string x64Asm(const std::string& code)
	{
		std::string bytecode;
		for (auto& line : string::explode(code, '\n'))
		{
			string::trim(line);
			if (line.empty())
			{
				continue;
			}
			x64Instruction ins;
			ins.fromString(line);
			bytecode.append(ins.toBytecode());
		}
		return bytecode;
	}

	x64Instruction x64Disasm(const uint8_t*& code)
	{
		bool operand_size_override = false;
		bool address_size_override = false;
		bool rex = false;
		bool default_operand_size = true;
		bool reg_x = false;
		bool rm_x = false;
		bool off_b_x = false;

		// "Mandatory" prefix
		if (*code == 0x67)
		{
			address_size_override = true;
			++code;
		}
		if (*code == 0x66)
		{
			operand_size_override = true;
			++code;
		}

		// REX prefix
		if ((*code >> 4) == 0b0100)
		{
			rex = true;
			if ((*code >> 3) & 1)
			{
				default_operand_size = false;
			}
			reg_x = ((*code >> 2) & 1);
			off_b_x = ((*code >> 1) & 1);
			rm_x = (*code & 1);
			++code;
		}

		// Opcode
		x64Instruction res{};
		for (const auto& op : operations)
		{
			if (op.matches(code))
			{
				if (*code == 0xF3)
				{
					++code;
				}
				if (*code == 0x0F)
				{
					++code;
				}
				res.operation = &op;
				if (op.operand_encoding != ZO)
				{
					uint8_t operand_size;
					uint8_t immediate_size;
					if (operand_size_override)
					{
						operand_size = immediate_size = 16;
					}
					else if (op.operand_size == 0)
					{
						operand_size = immediate_size = (default_operand_size ? 32 : 64);
					}
					else if (default_operand_size)
					{
						operand_size = immediate_size = op.operand_size;
					}
					else
					{
						operand_size = 64;
						immediate_size = op.operand_size;
					}

					uint8_t opcode = *code;
					uint8_t opr_i = 0;
					uint8_t num_modrm_oprs = op.getNumModrmOperands();
					bool modrm_read = false;
					uint8_t modrm;
					uint8_t sib;

					for (uint8_t opr_enc; opr_enc = ((op.operand_encoding >> (opr_i * BITS_PER_OPERAND)) & OPERAND_MASK), opr_enc != ZO; )
					{
						x64Operand& opr = res.operands[opr_i];
						if (opr_enc == O)
						{
							opr.decode(rex, operand_size, opcode & 0b111, rm_x);
						}
						else if (opr_enc == M || opr_enc == R)
						{
							const bool left = !modrm_read;
							if (left)
							{
								modrm_read = true;
								modrm = *++code;
							}
							const bool direct = ((modrm >> 6) == 0b11);
							if (opr_enc == M)
							{
								opr.decode(rex || !direct, operand_size, modrm & 0b111, rm_x);
								if (!direct)
								{
									opr.access_type = (address_size_override ? ACCESS_32 : ACCESS_64);
									opr.deref_size = ((num_modrm_oprs == 2) ? 1 : operand_size); // Hiding pointer type when other operand makes it apparent
									if ((modrm & 0b111) == SP)
									{
										sib = *++code;
									}
									if ((modrm >> 6) == 0b01)
									{
										opr.deref_offset = (int8_t)*++code;
									}
									else if ((modrm >> 6) == 0b10)
									{
										getImmediate(opr.deref_offset, code, 4);
									}
									else if ((modrm >> 6) == 0b00)
									{
										if ((modrm & 0b111) == SP)
										{
											if (sib == 0x25)
											{
												// Absolute address
												opr.reg = IMM;
												getImmediate(opr.val, code, 4);
											}
											else if (sib != 0x24)
											{
												// Offset
												opr.reg = OFF;
												opr.off.a = (x64Register)((sib & 0b111) | (rm_x << 3));
												opr.off.b = (x64Register)(((sib >> 3) & 0b111) | (off_b_x << 3));
											}
										}
										else if (opr.reg == BP)
										{
											// RIP-relative
											opr.reg = IP;
											getImmediate(opr.deref_offset, code, 4);
										}
									}
								}
							}
							else if (opr_enc == R)
							{
								opr.decode(rex, operand_size, (modrm >> 3) & 0b111, reg_x);
							}
						}
						else if (opr_enc == I)
						{
							opr.reg = IMM;
							getImmediate(opr.val, code, immediate_size / 8);
						}
						else if (opr_enc == D)
						{
							opr.reg = DIS;
							getImmediate(opr.val, code, immediate_size / 8);
							if (immediate_size == 32)
							{
								opr.displacement = (int64_t)(int32_t)(uint32_t)opr.val;
							}
							else if (immediate_size == 8)
							{
								opr.displacement = (int64_t)(int8_t)(uint8_t)opr.val;
							}
						}
						else if (opr_enc == A)
						{
							opr.decode(rex, operand_size, RA, reg_x);
						}

						if (++opr_i == COUNT(res.operands))
						{
							break;
						}
					}
				}
				++code;
				break;
			}
		}
		return res;
	}

	uint8_t x64GetLength(const uint8_t* code)
	{
		auto start = code;
		return x64Disasm(code).isValid()
			? (uint8_t)(code - start)
			: 0;
	}

	const uint8_t* x64GetPrev(const uint8_t* code)
	{
		const auto present = code;
		for (uint8_t i = 0; i != 100; ++i)
		{
			--code;
			auto start = code;
			if (x64IsStartByte(code)
				&& x64GetNext(code) == present
				)
			{
				return start;
			}
		}
		return nullptr;
	}

	const uint8_t* x64GetNext(const uint8_t* code)
	{
		return x64Disasm(code).isValid()
			? code
			: nullptr
			;
	}

	bool x64IsStartByte(const uint8_t* code)
	{
		return *code == 0x0F // 2-byte opcode
			|| (*code & 0xF0) == 0x40 // REX
			|| (
				(((*code & 0xF0) == 0x50) // push or pop
					|| *code == 0x33 // or xor
				)
				&& ((*(code - 1) & 0xF0) != 0x40) // and no REX before
			)
			;
	}
}
