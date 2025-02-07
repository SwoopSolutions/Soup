#pragma once

#include <string>
#include <utility> // pair
#include <vector>

#include "rflFunc.hpp"
#include "rflParser.hpp"
#include "string.hpp"

NAMESPACE_SOUP
{
	/*
	std::ofstream out(R"(C:\Users\Sainan\Desktop\Repos\Soup\bindings\cwraps.js)");
	out << CApiUtil::getCwraps(CApiUtil::parseFunctions(string::fromFile(R"(C:\Users\Sainan\Desktop\Repos\Soup\bindings\soup.h)")));
	*/
	struct CApiUtil
	{
		[[nodiscard]] static std::vector<std::pair<std::string, std::vector<rflFunc>>> parseFunctions(const std::string& soup_h)
		{
			std::vector<std::pair<std::string, std::vector<rflFunc>>> ret{};

			bool begin = false;
			std::string ns{};
			std::vector<rflFunc> funcs{};
			for (auto& line : string::explode(soup_h, '\n'))
			{
				if (!line.empty() && line.back() == '\r')
				{
					line.pop_back();
				}
				if (!begin)
				{
					if (line == "// [global namespace]")
					{
						begin = true;
					}
					continue;
				}

				if (line.substr(0, 3) == "// ")
				{
					ret.emplace_back(std::move(ns), std::move(funcs));
					ns = line.substr(3);
				}
				else if (line.substr(0, 13) == "SOUP_CEXPORT ")
				{
					rflParser p(line.substr(13));
					funcs.emplace_back(p.readFunc());
				}
			}
			ret.emplace_back(std::move(ns), std::move(funcs));

			return ret;
		}

		[[nodiscard]] static std::string getJsType(const rflType& type)
		{
			if (type.at == rflType::POINTER)
			{
				if (type.name == "const char")
				{
					return "string";
				}
			}
			else
			{
				if (type.name == "void_func_t")
				{
					return "function";
				}
				if (type.name == "bool")
				{
					return "boolean"; // It has to be "boolean", not "bool", otherwise emscripten interprets it as "number".
				}
				if (type.name == "void")
				{
					return type.name;
				}
			}
			return "number";
		}

		[[nodiscard]] static std::string getCwrap(const rflFunc& f)
		{
			std::string str = "soup.cwrap(\"";
			str.append(f.name);
			str.append("\", \"");
			str.append(getJsType(f.return_type));
			str.append("\", [");
			for (auto i = f.parameters.begin(); i != f.parameters.end(); ++i)
			{
				if (i != f.parameters.begin())
				{
					str.append(", ");
				}
				str.push_back('"');
				str.append(getJsType(i->type));
				str.push_back('"');
			}
			str.append("])");
			return str;
		}

		[[nodiscard]] static std::string getCwraps(const std::vector<std::pair<std::string, std::vector<rflFunc>>>& vec)
		{
			std::string str;
			for (const auto& ns : vec)
			{
				if (ns.first.empty())
				{
					for (const auto& f : ns.second)
					{
						str.append("\t\t\tsoup.");
						str.append(f.name == "endLifetime" ? "free" : f.name);
						str.append(" = ");
						str.append(getCwrap(f));
						str.append(";\n");
					}
				}
				else
				{
					str.append("\t\t\tsoup.");
					str.append(ns.first);
					str.append(" = {\n");
					for (const auto& f : ns.second)
					{
						str.append("\t\t\t\t");
						str.append(f.name.substr(ns.first.size() + 1));
						str.append(": ");
						str.append(getCwrap(f));
						str.append(",\n");
					}
					str.append("\t\t\t};\n");
				}
			}
			return str;
		}
	};
}
