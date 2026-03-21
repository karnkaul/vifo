#include "vifo/formatter.hpp"
#include "detail/formatter/pattern_swapper.hpp"
#include <djson/json.hpp>

namespace vifo {
auto PatternSwapFormat::from_file(std::string_view const path) -> std::optional<PatternSwapFormat> {
	auto const result = dj::Json::from_file(path);
	if (!result) { return {}; }

	auto const& json = *result;
	auto ret = PatternSwapFormat{};
	from_json(json["input"], ret.input);
	from_json(json["output"], ret.output);
	if (ret.input.empty() || ret.output.empty()) { return {}; }

	return ret;
}
} // namespace vifo

auto vifo::create_pattern_swapper(PatternSwapFormat format) -> Result<std::unique_ptr<IFormatter>> {
	auto ret = std::make_unique<detail::PatternSwapper>();
	return ret->initialize(std::move(format)).transform([&] { return std::move(ret); });
}
