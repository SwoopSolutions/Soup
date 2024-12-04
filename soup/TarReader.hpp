#pragma once

#include "Reader.hpp"
#include "TarIndexedFile.hpp"

NAMESPACE_SOUP
{
	struct TarReader
	{
		Reader& r;

		TarReader(Reader& r) noexcept
			: r(r)
		{
		}

		[[nodiscard]] std::vector<TarIndexedFile> getFileList() const SOUP_EXCAL;

		[[nodiscard]] std::string getFileContents(const TarIndexedFile& file) const SOUP_EXCAL
		{
			r.seek(file.offset);
			std::string res(file.size, '\0');
			r.raw(res.data(), file.size);
			return res;
		}
	};
}
