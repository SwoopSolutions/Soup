#pragma once

#include "dnsResolver.hpp"

namespace soup
{
	struct dnsSmartResolver : public dnsResolver
	{
		IpAddr server = SOUP_IPV4(1, 1, 1, 1);

		[[nodiscard]] UniquePtr<dnsLookupTask> makeLookupTask(dnsType qtype, const std::string& name) const final;

		mutable UniquePtr<dnsResolver> subresolver{};
		mutable bool tested_udp = false;
	};
}
