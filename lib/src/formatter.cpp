#include "vifo/formatter.hpp"
#include "detail/formatter/interpolator.hpp"
#include <djson/json.hpp>

namespace vifo {
auto InterpolateFormat::from_file(std::string_view const path) -> std::optional<InterpolateFormat> {
	auto const result = dj::Json::from_file(path);
	if (!result) { return {}; }

	auto const& json = *result;
	auto ret = InterpolateFormat{};
	from_json(json["input"], ret.input);
	from_json(json["output"], ret.output);
	if (ret.input.empty() || ret.output.empty()) { return {}; }

	return ret;
}
} // namespace vifo

auto vifo::create_interpolator(InterpolateFormat format) -> Result<std::unique_ptr<IFormatter>> {
	auto ret = std::make_unique<detail::Interpolator>();
	return ret->initialize(std::move(format)).transform([&] { return std::move(ret); });
}
