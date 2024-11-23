#include "Hotp.hpp"

#include "math.hpp"
#include "rand.hpp"
#include "sha1.hpp"
#include "StringWriter.hpp"

NAMESPACE_SOUP
{
	std::string Hotp::generateSecret(size_t bytes)
	{
		std::string secret;
		secret.reserve(bytes);
		while (bytes--)
		{
			secret.push_back(soup::rand.byte());
		}
		return secret;
	}

	int Hotp::getValue(uint64_t counter) const
	{
		return getValueRaw(counter) % 1000000;
	}

	int Hotp::getValue(uint64_t counter, uint8_t digits) const
	{
		return getValueRaw(counter) % soup::pow<int>(10, digits);
	}

	int Hotp::getValueRaw(uint64_t counter) const
	{
		StringWriter w;
		w.u64be(counter);

		std::string mac = sha1::hmac(w.data, secret);

		auto offset = ((uint8_t)mac.at(19) & 0xf);
		int bin_code = ((uint8_t)mac.at(offset) & 0x7f) << 24
			| ((uint8_t)mac.at(offset + 1) & 0xff) << 16
			| ((uint8_t)mac.at(offset + 2) & 0xff) << 8
			| ((uint8_t)mac.at(offset + 3) & 0xff);

		return bin_code;
	}
}

