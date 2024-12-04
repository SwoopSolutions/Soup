#include "CidrSubnetInterface.hpp"

#include "CidrSubnet4Interface.hpp"
#include "CidrSubnet6Interface.hpp"
#include "IpAddr.hpp"
#include "string.hpp"

NAMESPACE_SOUP
{
	UniquePtr<CidrSubnetInterface> CidrSubnetInterface::construct(const std::string& str)
	{
		auto sep = str.find('/');
		IpAddr addr;
		SOUP_ASSERT(addr.fromString(str.substr(0, sep)));
		auto size = string::toIntOpt<uint8_t>(str.substr(sep + 1), string::TI_FULL);
		return construct(addr, size.value());
	}

	UniquePtr<CidrSubnetInterface> CidrSubnetInterface::construct(const IpAddr& addr, uint8_t size)
	{
		if (addr.isV4())
		{
			return soup::make_unique<CidrSubnet4Interface>(addr, size);
		}
		return soup::make_unique<CidrSubnet6Interface>(addr, size);
	}

	bool CidrSubnetInterface::contains(const CidrSubnetInterface& b) const noexcept
	{
		return getSize() >= b.getSize() && contains(b.getAddr());
	}

	std::string CidrSubnetInterface::toString() const
	{
		std::string str = getAddr().toString();
		str.push_back('/');
		str.append(std::to_string(getSize()));
		return str;
	}
}
