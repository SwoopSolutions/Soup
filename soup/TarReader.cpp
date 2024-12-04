#include "TarReader.hpp"

#include "string.hpp"

NAMESPACE_SOUP
{
	std::vector<TarIndexedFile> TarReader::getFileList() const SOUP_EXCAL
	{
		r.seekEnd();
		const size_t tarsize = r.getPosition();
		SOUP_IF_UNLIKELY ((tarsize % 512) != 0)
		{
			return {}; // Not a valid tarball
		}

		std::vector<TarIndexedFile> res{};
		for (size_t i = 0; i != tarsize; )
		{
			r.seek(i + 124);
			char size_octal[13];
			memset(size_octal, 0, sizeof(size_octal));
			r.raw(size_octal, 12);
			size_t size;
			SOUP_IF_UNLIKELY (!string::toIntEx<size_t, 8>(size_octal, string::TI_FULL).consume(size))
			{
				break; // Tarball might end on a null block
			}

			r.seek(i + 156);
			char type;
			r.c(type);
			if (type == '\0' || type == '0') // Regular file?
			{
				auto& file = res.emplace_back();

				memset(file.name, 0, sizeof(file.name));
				r.seek(i);
				r.raw(file.name, 100);

				file.offset = i + 512;
				file.size = size;
			}

			i += ((1 + ((size + 511) / 512)) * 512);
		}
		return res;
	}
}
