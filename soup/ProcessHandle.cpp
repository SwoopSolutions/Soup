#include "ProcessHandle.hpp"

#include "Range.hpp"

#if SOUP_WINDOWS
	#include <windows.h>
#else
	#include "FileReader.hpp"
	#include "Regex.hpp"
	#include "string.hpp"
#endif

NAMESPACE_SOUP
{
	std::vector<Range> ProcessHandle::getAllocations() const
	{
		std::vector<Range> res{};
#if SOUP_WINDOWS
		MEMORY_BASIC_INFORMATION mbi{};

		PBYTE addr = NULL;
		while (VirtualQueryEx(*h, addr, &mbi, sizeof(mbi)) == sizeof(mbi))
		{
			if (mbi.State == MEM_COMMIT)
			{
				res.emplace_back(mbi.BaseAddress, mbi.RegionSize);
			}

			addr = (PBYTE)mbi.BaseAddress + mbi.RegionSize;
		}
#else
		Regex r(R"(^(?'start'[0-9A-Fa-f]+)-(?'end'[0-9A-Fa-f]+) +(?'prots'[a-z\-]+) +[^ ]+ +[^ ]+ +[^ ]+ +(?'mappedfile'.*)$)");
		FileReader fr("/proc/" + std::to_string(pid) + "/maps");
		for (std::string line; fr.getLine(line); )
		{
			auto m = r.match(line);
			if (m.findGroupByName("prots")->begin[0] != 'r')
			{
				continue;
			}
			if (auto mappedfile = m.findGroupByName("mappedfile"))
			{
				if (mappedfile->length() != 0)
				{
					continue;
				}
			}
			auto start = string::hexToInt<uintptr_t>(m.findGroupByName("start")->toString()).value();
			auto end = string::hexToInt<uintptr_t>(m.findGroupByName("end")->toString()).value();
			res.emplace_back(start, end - start);
		}
#endif
		return res;
	}
}
