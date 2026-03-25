#include "vifo/json_io.hpp"
#include "djson/json.hpp"
#include "vifo/omdb.hpp"
#include "vifo/util/util.hpp"
#include <string_view>

namespace vifo {
namespace {
namespace key {
constexpr std::string_view title_v{"Title"};
constexpr std::string_view year_v{"Year"};
constexpr std::string_view imdb_id_v{"imdbID"};
constexpr std::string_view plot_v{"Plot"};
constexpr std::string_view episode_v{"Episode"};
constexpr std::string_view season_v{"Season"};
constexpr std::string_view episodes_v{"Episodes"};
constexpr std::string_view total_seasons_v{"totalSeasons"};
} // namespace key

void from_string(dj::Json const& in, int& out) {
	auto text = in.as_string_view();
	auto end_index = text.size();
	if (auto const i = text.find('-'); i > 0 && i < std::string_view::npos) { end_index = i; }
	out = util::to_int(text.substr(0, end_index));
}
} // namespace
} // namespace vifo

void vifo::from_json(dj::Json const& json, omdb::Movie& out) {
	from_json(json[key::title_v], out.title);
	from_json(json[key::imdb_id_v], out.imdb_id);
	from_string(json[key::year_v], out.year);
	from_json(json[key::plot_v], out.plot);
}

void vifo::from_json(dj::Json const& json, omdb::Episode& out) {
	from_string(json[key::episode_v], out.number);
	from_json(json[key::title_v], out.title);
	from_json(json[key::imdb_id_v], out.imdb_id);
	from_json(json[key::plot_v], out.plot);
}

void vifo::from_json(dj::Json const& json, omdb::Season& out) {
	from_string(json[key::season_v], out.number);
	from_json(json[key::title_v], out.title);
	for (auto const& in : json[key::episodes_v].as_array()) { from_json(in, out.episodes.emplace_back()); }
}

void vifo::from_json(dj::Json const& json, omdb::Series& out) {
	from_json(json[key::title_v], out.title);
	from_string(json[key::year_v], out.year);
	from_json(json[key::imdb_id_v], out.imdb_id);
	from_json(json[key::plot_v], out.plot);
	from_string(json[key::total_seasons_v], out.total_seasons);
}

void vifo::from_json(dj::Json const& json, interpolator::PatternSwap::Format& format) {
	from_json(json["input"], format.input);
	from_json(json["output"], format.output);
}

void vifo::to_json(dj::Json& json, interpolator::PatternSwap::Format const& format) {
	to_json(json["input"], format.input);
	to_json(json["output"], format.output);
}

void vifo::from_json(dj::Json const& json, formatter::PatternSwap::Format& format) {
	from_json(json["directory"], format.directory);
	if (auto const& file = json["file"]) {
		format.file.emplace();
		from_json(file, *format.file);
	}
}

void vifo::to_json(dj::Json& json, formatter::PatternSwap::Format const& format) {
	to_json(json["directory"], format.directory);
	if (format.file) { to_json(json["file"], *format.file); }
}
