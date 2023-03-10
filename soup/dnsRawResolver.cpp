#include "dnsRawResolver.hpp"

#include "dnsHeader.hpp"
#include "dnsQuestion.hpp"
#include "dnsResource.hpp"
#include "string.hpp"
#include "StringWriter.hpp"

namespace soup
{
	std::string dnsRawResolver::getQuery(dnsType qtype, const std::string& name)
	{
		StringWriter sw(false);

		dnsHeader dh{};
		dh.setRecursionDesired(true);
		dh.qdcount = 1;
		dh.write(sw);

		dnsQuestion dq;
		dq.name.name = string::explode(name, '.');
		dq.qtype = qtype;
		dq.write(sw);

		return sw.data;
	}

	std::vector<UniquePtr<dnsRecord>> dnsRawResolver::parseResponse(std::string&& data)
	{
		StringReader sr(std::move(data), false);

		dnsHeader dh;
		dh.read(sr);

		for (uint16_t i = 0; i != dh.qdcount; ++i)
		{
			dnsQuestion dq;
			dq.read(sr);
		}

		std::vector<UniquePtr<dnsRecord>> res{};
		for (uint16_t i = 0; i != dh.ancount; ++i)
		{
			dnsResource dr;
			dr.read(sr);

			std::string name = string::join(dr.name.resolve(sr.data), '.');

			if (dr.rclass != DNS_IN)
			{
				continue;
			}

			if (dr.rtype == DNS_A)
			{
				res.emplace_back(soup::make_unique<dnsARecord>(std::move(name), dr.ttl, *reinterpret_cast<uint32_t*>(dr.rdata.data())));
			}
			else if (dr.rtype == DNS_AAAA)
			{
				res.emplace_back(soup::make_unique<dnsAaaaRecord>(std::move(name), dr.ttl, reinterpret_cast<uint8_t*>(dr.rdata.data())));
			}
			else if (dr.rtype == DNS_CNAME)
			{
				StringReader rdata_sr(std::move(dr.rdata), false);

				dnsName cname;
				cname.read(rdata_sr);

				res.emplace_back(soup::make_unique<dnsCnameRecord>(std::move(name), dr.ttl, string::join(cname.resolve(sr.data), '.')));
			}
			else if (dr.rtype == DNS_PTR)
			{
				StringReader rdata_sr(std::move(dr.rdata), false);

				dnsName cname;
				cname.read(rdata_sr);

				res.emplace_back(soup::make_unique<dnsPtrRecord>(std::move(name), dr.ttl, string::join(cname.resolve(sr.data), '.')));
			}
			else if (dr.rtype == DNS_TXT)
			{
				std::string data;
				for (size_t i = 0; i != dr.rdata.size(); )
				{
					auto chunk_size = (uint8_t)dr.rdata.at(i++);
					data.append(dr.rdata.substr(i, chunk_size));
					i += chunk_size;
				}
				res.emplace_back(soup::make_unique<dnsTxtRecord>(std::move(name), dr.ttl, std::move(data)));
			}
		}
		return res;
	}
}
