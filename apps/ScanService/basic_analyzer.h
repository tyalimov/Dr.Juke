#pragma once

#include <json.hpp>
#include <filesystem>
#include <string>

namespace drjuke::scansvc
{
    // TODO: Перенести в коммон
	using Json          = nlohmann::json;
	using Path          = std::filesystem::path;
	using AnalyzeReport = Json;

	class BasicAnalyzer
	{
	public:
		virtual ~BasicAnalyzer()                        = default;
		virtual AnalyzeReport analyze(const Path &path) = 0;
		virtual void prepare()                          = 0;
		virtual std::string getName()           = 0;
	};
}