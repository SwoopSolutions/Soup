#pragma once

#include "base.hpp"
#include "fwd.hpp"
#include "type.hpp"

#include <string>
#include <vector>

#if SOUP_WINDOWS
#include <windows.h>
#include <winternl.h>
#endif

NAMESPACE_SOUP
{
	class os
	{
	public:
		static void escape(std::string& str);
	private:
		static void escapeNoCheck(std::string& str);
	public:
		static std::string execute(std::string program, const std::vector<std::string>& args = {});
		static std::string executeLong(std::string program, const std::vector<std::string>& args = {});
	private:
		static void resolveProgram(std::string& program);
		static std::string executeInner(std::string program, const std::vector<std::string>& args);
	public:

		[[nodiscard]] static UniquePtr<AllocRaiiVirtual> allocateExecutable(const std::string& bytecode);
		[[nodiscard]] static UniquePtr<AllocRaiiVirtual> allocateExecutable(const std::vector<uint8_t>& bytecode);
		[[nodiscard]] static void* virtualAlloc(size_t len, int prot);
		static void virtualFree(void* addr, size_t len);
		static void changeProtection(void* addr, size_t len, int prot);

		[[nodiscard]] static pid_t getProcessId() noexcept;

#if SOUP_WINDOWS
		static bool copyToClipboard(const std::string& text);

		[[nodiscard]] static size_t getMemoryUsage();

		[[nodiscard]] static bool isWine();

		[[nodiscard]] static PEB* getCurrentPeb();

		[[nodiscard]] static std::string makeScreenshotBmp(int x, int y, int width, int height);

		[[nodiscard]] static int getPrimaryScreenWidth()
		{
			return GetSystemMetrics(SM_CXSCREEN);
		}

		[[nodiscard]] static int getPrimaryScreenHeight()
		{
			return GetSystemMetrics(SM_CYSCREEN);
		}
#endif
	};
}
