#pragma once

#include "base.hpp"

#include <cstdint>
#include <cstddef>
#include <unordered_set>
#include <vector>

#include "HidUsage.hpp"

NAMESPACE_SOUP
{
	struct HidParsedReport
	{
		std::unordered_set<HidUsage> active_selectors{};
	};

	struct HidReportDescriptor
	{
		uint16_t usage_page = 0;
		uint16_t usage = 0;
		uint16_t input_report_byte_length = 0; // including report id
		uint16_t output_report_byte_length = 0; // including report id
		uint16_t feature_report_byte_length = 0; // including report id
		std::unordered_set<uint8_t> report_ids{};

		struct ReportField
		{
			uint32_t size;
			uint32_t count;
			bool is_variable;
			uint16_t usage_page;
			std::vector<uint16_t> usage_ids;
		};
		std::vector<ReportField> input_report_fields{};

		[[nodiscard]] static HidReportDescriptor parse(const void* _rawdesc, size_t size);

		[[nodiscard]] HidParsedReport parseInputReport(const void* report, size_t size) const;
	};
}
