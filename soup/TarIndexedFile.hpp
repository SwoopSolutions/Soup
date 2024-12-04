#pragma once

#include "base.hpp"

NAMESPACE_SOUP
{
	struct TarIndexedFile
	{
		char name[101];
		size_t offset;
		size_t size;
	};
}
