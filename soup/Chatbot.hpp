#pragma once

#include "fwd.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace soup
{
	struct Chatbot
	{
		[[nodiscard]] static const std::vector<UniquePtr<cbCmd>>& getAllCommands();

		static void intialiseResources(std::filesystem::path dir);

		[[nodiscard]] static cbResult process(const std::string& text);
		[[nodiscard]] static std::string getResponse(const std::string& text);
	};
}
