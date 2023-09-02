#pragma once

#include "dnsResolver.hpp"

namespace soup
{
	struct dnsOsResolver : public dnsResolver
	{
		[[nodiscard]] std::vector<UniquePtr<dnsRecord>> lookup(dnsType qtype, const std::string& name) const final;
	};
}
