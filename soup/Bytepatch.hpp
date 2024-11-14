#pragma once

#include <cstddef> // size_t
#include <cstdint>

#include "base.hpp"

NAMESPACE_SOUP
{
	class Bytepatch
	{
	public:
		enum Instructions : uint8_t
		{
			NOP = 0x90,
			RET = 0xC3,
			JMP1 = 0xEB,
		};

	private:
		uint8_t* og_area = nullptr;
		size_t size = 0;
		uint8_t* area = nullptr;

	public:
		explicit constexpr Bytepatch() = default;
		~Bytepatch() noexcept;

		Bytepatch(Bytepatch&& b) noexcept;
		void operator =(Bytepatch&& b) noexcept;
	private:
		void patchFrom(Bytepatch&& b) noexcept;

	public:
		[[nodiscard]] bool isPatched() const noexcept;
	private:
		void forget() noexcept;
		void store() SOUP_EXCAL;
		void store(uint8_t* area, size_t size) SOUP_EXCAL;
	public:
		void initPatch(uint8_t* area, const uint8_t* patch, size_t size) SOUP_EXCAL;
		bool initPatchNOP(uint8_t* area, size_t size) SOUP_EXCAL;
		void initPatchZero(uint8_t* area, size_t size) SOUP_EXCAL;
		void initPatchZero() SOUP_EXCAL;
		void restore() noexcept;
		void dispose() noexcept;
	};
}
