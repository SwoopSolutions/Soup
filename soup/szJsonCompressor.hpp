#pragma once

#include "szCompressor.hpp"

#include "json.hpp"

NAMESPACE_SOUP
{
	struct szJsonCompressor : public szCompressor
	{
		[[nodiscard]] szMethod getMethod() const noexcept final
		{
			return SM_JSON;
		}

		[[nodiscard]] bool isByteAligned() const noexcept final
		{
			return true;
		}

		void compress(BitWriter& bw, const std::string& data) const final
		{
			if (auto jt = json::decode(data))
			{
				jt->binaryEncode(bw.getStream());
			}
		}

		[[nodiscard]] szPreservationLevel getPreservationLevel(const szCompressResult& res, const std::string& data) const final
		{
			if (res.data.getBitLength() == 0)
			{
				return NONE;
			}
			if (checkDecompressed(res, data))
			{
				return LOSSLESS;
			}
			return LOSSY;
		}

		[[nodiscard]] std::string decompress(BitReader& br) const final
		{
			auto jt = json::binaryDecode(br.getStream());
			return jt->encodePretty();
		}
	};
}
