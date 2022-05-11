#include "JsonFloat.hpp"

#include <string>

#include "Writer.hpp"

namespace soup
{
	JsonFloat::JsonFloat(double value) noexcept
		: JsonNode(JSON_FLOAT), value(value)
	{
	}

	std::string JsonFloat::encode() const
	{
		return std::to_string(value);
	}

	bool JsonFloat::binaryEncode(Writer& w) const
	{
		uint8_t b = JSON_FLOAT;
		uint64_t val;
		*reinterpret_cast<double*>(&val) = value;
		return w.u8(b)
			&& w.u64(val)
			;
	}
}
