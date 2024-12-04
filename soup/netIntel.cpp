#include "netIntel.hpp"

#include "bitutil.hpp"
#include "CidrSubnet4Interface.hpp"
#include "CidrSubnet6Interface.hpp"
#include "csv.hpp"
#include "deflate.hpp"
#include "FileWriter.hpp"
#include "Ipv6Maths.hpp"
#include "netIntelLocationData4OnDisk.hpp"
#include "string.hpp"
#include "StringReader.hpp"
#include "wasm.hpp"
#include "WebResource.hpp"

NAMESPACE_SOUP
{
	void netIntel::init(bool ipv4, bool ipv6)
	{
		asInit(ipv4, ipv6);
		locationInit(ipv4, ipv6);
	}

	void netIntel::deinit() noexcept
	{
		asDeinit();
		locationDeinit();
	}

	void netIntel::asInit(bool ipv4, bool ipv6)
	{
		initAsList();
		initExtraWasm();
		if (ipv4)
		{
			initIpv4ToAs();
		}
		if (ipv6)
		{
			initIpv6ToAs();
		}
	}

	bool netIntel::asIsInited() noexcept
	{
		return !as_pool.empty();
	}

	void netIntel::asDeinit() noexcept
	{
		as_pool.clear();
		aslist.clear();
		ipv4toas.clear();
		ipv6toas.clear();
	}

	void netIntel::locationInit(bool ipv4, bool ipv6)
	{
		if (ipv4)
		{
			initIpv4ToLocation();
		}
		if (ipv6)
		{
			initIpv6ToLocation();
		}
	}

	bool netIntel::locationIsInited() noexcept
	{
		return !location_pool.empty();
	}

	void netIntel::locationDeinit() noexcept
	{
		location_pool.clear();
		ipv4tolocation.clear();
		ipv6tolocation.clear();
	}

	void netIntel::initAsList()
	{
		StringReader aslistcsv;
		{
			WebResource rsc("raw.githubusercontent.com", "/ipverse/asn-info/master/as.csv");
			rsc.downloadWithCaching();
			aslistcsv = std::move(rsc.data);
		}
		std::string line;
		aslistcsv.getLine(line); // skip field names
		while (aslistcsv.getLine(line))
		{
			auto asn_sep = line.find(',');
			SOUP_IF_UNLIKELY (asn_sep == std::string::npos)
			{
				continue;
			}
			netAs as;
			as.number = string::toIntOpt<uint32_t>(line.substr(0, asn_sep)).value();
			++asn_sep;
			auto handle_sep = line.find(',', asn_sep);
			as.handle = as_pool.emplace(line.substr(asn_sep, handle_sep - asn_sep));
			as.name = as_pool.emplace(line.substr(handle_sep + 2, line.length() - handle_sep - 3));
			aslist.emplace(as.number, soup::make_unique<netAs>(std::move(as)));
		}
	}

	void netIntel::initExtraWasm()
	{
		WebResource rsc("raw.githubusercontent.com", "/calamity-inc/soup-extra-data/senpai/build/release.wasm");
		rsc.downloadWithCaching();
		extra_wasm = std::move(rsc.data);

		// Apply AS name overwrites from https://github.com/calamity-inc/soup-extra-data/blob/senpai/index.blume
		WasmScript ws;
		SOUP_IF_LIKELY (ws.load(extra_wasm))
		{
			if (auto code = ws.getExportedFuntion("asn_name_overwrites"))
			{
				WasmVm vm(ws);
				if (vm.run(*code))
				{
					while (!vm.stack.empty())
					{
						auto name = vm.stack.top(); vm.stack.pop();
						auto asn = vm.stack.top(); vm.stack.pop();
						if (auto e = aslist.find(asn.i32); e != aslist.end())
						{
							e->second->name = as_pool.emplace(ws.getMemory<const char>(name));
						}
					}
				}
			}
		}
	}

	void netIntel::initIpv4ToAs()
	{
		StringReader ipv4toasntsv;
		{
			WebResource rsc("iptoasn.com", "/data/ip2asn-v4-u32.tsv.gz");
			rsc.downloadWithCaching();
			ipv4toasntsv = deflate::decompress(std::move(rsc.data)).decompressed;
		}
		for (std::string line; ipv4toasntsv.getLine(line); )
		{
			auto arr = string::explode(line, '\t');
			SOUP_IF_UNLIKELY (arr.size() < 5)
			{
				continue;
			}
			uint32_t asn = string::toIntOpt<uint32_t>(arr.at(2)).value();
			if (asn == 0)
			{
				continue;
			}
			auto begin = string::toIntOpt<uint32_t>(arr.at(0)).value();
			auto end = string::toIntOpt<uint32_t>(arr.at(1)).value();
			const netAs* as = getAsByNumber(asn);
			if (as == nullptr)
			{
				as = aslist.emplace(asn, soup::make_unique<netAs>(asn, as_pool.emplace(std::move(arr.at(4))))).first->second.get();
			}
			ipv4toas.emplace(begin, end, as);
		}
	}

	void netIntel::initIpv6ToAs()
	{
		StringReader ipv6toasntsv;
		{
			WebResource rsc("iptoasn.com", "/data/ip2asn-v6.tsv.gz");
			rsc.downloadWithCaching();
			ipv6toasntsv = deflate::decompress(std::move(rsc.data)).decompressed;
		}
		for (std::string line; ipv6toasntsv.getLine(line); )
		{
			auto arr = string::explode(line, '\t');
			SOUP_IF_UNLIKELY (arr.size() < 5)
			{
				continue;
			}
			uint32_t asn = string::toIntOpt<uint32_t>(arr.at(2)).value();
			if (asn == 0)
			{
				continue;
			}
			IpAddr begin, end;
			SOUP_ASSERT(begin.fromString(arr.at(0)));
			SOUP_ASSERT(end.fromString(arr.at(1)));
			const netAs* as = getAsByNumber(asn);
			if (as == nullptr)
			{
				as = aslist.emplace(asn, soup::make_unique<netAs>(asn, as_pool.emplace(std::move(arr.at(4))))).first->second.get();
			}
			ipv6toas.emplace(std::move(begin), std::move(end), as);
		}
	}

	void netIntel::initIpv4ToLocation()
	{
		StringReader ipv4tolocationcsv;
		{
			WebResource rsc("raw.githubusercontent.com", "/sapics/ip-location-db/master/geolite2-city/geolite2-city-ipv4-num.csv.gz");
			rsc.downloadWithCaching();
			ipv4tolocationcsv = deflate::decompress(std::move(rsc.data)).decompressed;
		}
		ipv4tolocation.reserve(2'800'000);
		for (std::string line; ipv4tolocationcsv.getLine(line); )
		{
			auto arr = csv::parseLine(line);
			SOUP_IF_UNLIKELY (arr.size() < 6)
			{
				continue;
			}
			ipv4tolocation.emplace(
				string::toIntOpt<uint32_t>(arr.at(0)).value(),
				string::toIntOpt<uint32_t>(arr.at(1)).value(),
				netIntelLocationData{
					std::move(arr.at(2)),
					location_pool.emplace(std::move(arr.at(3))),
					location_pool.emplace(std::move(arr.at(5))),
				}
			);
		}
	}

	void netIntel::initIpv6ToLocation()
	{
		StringReader ipv6tolocationcsv;
		{
			WebResource rsc("raw.githubusercontent.com", "/sapics/ip-location-db/master/geolite2-city/geolite2-city-ipv6.csv.gz");
			rsc.downloadWithCaching();
			ipv6tolocationcsv = deflate::decompress(std::move(rsc.data)).decompressed;
		}
		ipv6tolocation.reserve(700'000);
		for (std::string line; ipv6tolocationcsv.getLine(line); )
		{
			auto arr = csv::parseLine(line);
			SOUP_IF_UNLIKELY (arr.size() < 6)
			{
				continue;
			}
			IpAddr begin, end;
			SOUP_ASSERT(begin.fromString(arr.at(0)));
			SOUP_ASSERT(end.fromString(arr.at(1)));
			ipv6tolocation.emplace(begin, end, netIntelLocationData{
				std::move(arr.at(2)),
				location_pool.emplace(std::move(arr.at(3))),
				location_pool.emplace(std::move(arr.at(5))),
			});
		}
	}

	const netAs* netIntel::getAsByNumber(uint32_t number) const noexcept
	{
		if (auto e = aslist.find(number); e != aslist.end())
		{
			return e->second.get();
		}
		return nullptr;
	}

	const netAs* netIntel::getAsByIp(const IpAddr& addr) const
	{
		return addr.isV4()
			? getAsByIpv4(addr.getV4NativeEndian())
			: getAsByIpv6(addr)
			;
	}

	const netAs* netIntel::getAsByIpv4(native_u32_t ip) const
	{
		if (auto e = ipv4toas.find(ip))
		{
			return *e;
		}
		return nullptr;
	}

	const netAs* netIntel::getAsByIpv6(const IpAddr& addr) const
	{
		if (auto e = ipv6toas.find(addr))
		{
			return *e;
		}
		return nullptr;
	}

	std::vector<UniquePtr<CidrSubnetInterface>> netIntel::getRangesByAs(const netAs* as) const
	{
		std::vector<UniquePtr<CidrSubnetInterface>> res;

		for (const auto& e : ipv4toas.data)
		{
			if (e.data == as)
			{
				UniquePtr<CidrSubnet4Interface> newElement = soup::make_unique<CidrSubnet4Interface>(
					IpAddr((native_u32_t)e.lower),
					static_cast<uint8_t>(31 - bitutil::getMostSignificantSetBit(/* e.upper - e.lower */ e.lower ^ e.upper))
				);

				auto it = res.begin();
				for (; it != res.end(); ++it)
				{
					if ((*it)->getAddr() == newElement->getAddr())
					{
						break;
					}
				}

				if (it != res.end())
				{
					if ((*it)->getSize() < newElement->getSize())
					{
						*it = std::move(newElement);
					}
				}
				else
				{
					res.emplace_back(std::move(newElement));
				}
			}
		}

		for (const auto& e : ipv6toas.data)
		{
			if (e.data == as)
			{
				auto delta = Ipv6Maths::fromIpAddr(e.lower);
				Ipv6Maths::xorEq(delta, Ipv6Maths::fromIpAddr(e.upper));

				UniquePtr<CidrSubnet6Interface> newElement = soup::make_unique<CidrSubnet6Interface>(
					e.lower,
					(127 - Ipv6Maths::getMostSignificantSetBit(delta))
				);

				auto it = res.begin();
				for (; it != res.end(); ++it)
				{
					if ((*it)->getAddr() == newElement->getAddr())
					{
						break;
					}
				}

				if (it != res.end())
				{
					if ((*it)->getSize() < newElement->getSize())
					{
						*it = std::move(newElement);
					}
				}
				else
				{
					res.emplace_back(std::move(newElement));
				}
			}
		}

		return res;
	}

	const netIntelLocationData* netIntel::getLocationByIp(const IpAddr& addr) const
	{
		return addr.isV4()
			? getLocationByIpv4(addr.getV4NativeEndian())
			: getLocationByIpv6(addr)
			;
	}
	
	const netIntelLocationData* netIntel::getLocationByIpv4(native_u32_t ip) const
	{
		return ipv4tolocation.find(ip);
	}

	const netIntelLocationData* netIntel::getLocationByIpv6(const IpAddr& addr) const
	{
		return ipv6tolocation.find(addr);
	}

	void netIntel::locationExport(const std::filesystem::path& dir)
	{
		std::unordered_map<const char*, uint32_t> offsets{};
		offsets.reserve(location_pool.pool.size());
		{
			FileWriter fw(dir / "location_pool.bin");
			fw.throwIfFailed();
			for (const auto& loc : location_pool.pool)
			{
				offsets.emplace(loc.c_str(), static_cast<uint32_t>(fw.s.tellp()));
				fw.str_nt(loc);
			}
		}
		
		{
			FileWriter fw(dir / "ipv4tolocation.bin");
			fw.throwIfFailed();
			for (const auto& e : ipv4tolocation.data)
			{
				netIntelLocationData4OnDisk data;
				data.lower = e.lower;
				data.upper = e.upper;

				data.country_code = e.data.country_code.c_str();
				data.state_offset = offsets.at(e.data.state);
				data.city_offset = offsets.at(e.data.city);

				data.write(fw);
			}
		}
	}
}
