#pragma once

#include "Packet.hpp"

#include "aud_common.hpp"

NAMESPACE_SOUP
{
	SOUP_PACKET(WavFmtChunk)
	{
		u16 format_tag;
		u16 channels;
		u32 sample_rate; // samples per second
		u32 avg_bytes_per_sec; // = (sample_rate * bytes_per_block)
		u16 bytes_per_block; // = ((bits_per_sample / 8) * channels)
		u16 bits_per_sample;

		SOUP_PACKET_IO(s)
		{
			return s.u16le(format_tag)
				&& s.u16le(channels)
				&& s.u32le(sample_rate)
				&& s.u32le(avg_bytes_per_sec)
				&& s.u16le(bytes_per_block)
				&& s.u16le(bits_per_sample)
				;
		}

		[[nodiscard]] bool isGoodForAudPlayback() const noexcept
		{
			return format_tag == 1 // WAVE_FORMAT_PCM
				&& (channels == 2 || channels == 1)
				&& sample_rate == AUD_SAMPLE_RATE
				&& bits_per_sample == 16
				;
		}
	};
}
