#include "vifo/media_file.hpp"
#include "klib/cli/text_table.hpp"
#include "klib/debug/assert.hpp"
#include "vifo/util/regex_matcher.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <ranges>
#include <vector>

namespace vifo {
namespace {
using util::RegexMatcher;

[[nodiscard]] constexpr auto starts_with_bracket(std::string_view const text) { return !text.empty() && (text.front() == '[' || text.front() == '('); }
[[nodiscard]] constexpr auto ends_with_bracket(std::string_view const text) { return !text.empty() && (text.back() == ']' || text.back() == ')'); }

[[nodiscard]] constexpr auto trim_brackets(std::string_view in) {
	while (starts_with_bracket(in)) { in.remove_prefix(1); }
	while (ends_with_bracket(in)) { in.remove_suffix(1); }
	return in;
}

auto const season_id_matcher = RegexMatcher{util::regex_season_id_v};
auto const episode_id_matcher = RegexMatcher{util::regex_episode_id_v};
auto const year_matcher = RegexMatcher{util::regex_year_v};
auto const resolution_matcher = RegexMatcher{util::regex_resolution_v};

[[nodiscard]] auto assign_if_match(std::string& out, std::string_view const word, RegexMatcher const& matcher) -> bool {
	auto match = matcher.get_match(word);
	if (match.empty()) { return false; }

	out = std::move(match);
	return true;
}

class MetadataBuilder {
  public:
	[[nodiscard]] auto operator()(std::string filename) -> MediaFile::Metadata {
		std::ranges::replace(filename, '.', ' ');

		for (auto const& it : std::views::split(filename, ' ')) {
			auto word = std::string_view{it};

			auto const bracketed = starts_with_bracket(word);
			if (bracketed) {
				word = trim_brackets(word);
				if (!m_ret.title.empty()) { m_skip_titles = true; }
			}

			if (is_next_season(word)) { continue; }
			if (is_season(word)) { continue; }
			if (is_regex_match(word)) { continue; }

			if (bracketed) { continue; }

			join_word(word);
		}
		return std::move(m_ret);
	}

  private:
	auto is_next_season(std::string_view const word) -> bool {
		if (!m_next_is_season) { return false; }

		m_next_is_season = false;
		m_ret.season_id = SeasonId{.number = util::to_int(word)};
		return true;
	}

	auto is_season(std::string_view word) -> bool {
		if (word.front() != 'S' && word.front() != 's') { return false; }
		word.remove_prefix(1);
		if (word != "eason") { return false; }

		m_next_is_season = true;
		return true;
	}

	[[nodiscard]] auto is_regex_match(std::string_view const word) -> bool {
		if (is_episode_id(word)) { return true; }

		if (!is_season_id(word) && !is_year(word) && !is_resolution(word)) { return false; }
		m_skip_titles = true;
		return true;
	}

	auto is_episode_id(std::string_view const word) -> bool {
		auto episode_id = std::string{};
		if (!assign_if_match(episode_id, word, episode_id_matcher)) { return false; }

		KLIB_ASSERT(episode_id.size() == std::string_view{"S00E00"}.size());
		auto const season = std::string_view{episode_id}.substr(1, 2);
		auto const episode = std::string_view{episode_id}.substr(4, 2);
		m_ret.episode_id = EpisodeId{.season = util::to_int(season), .number = util::to_int(episode)};
		return true;
	}

	auto is_season_id(std::string_view const word) -> bool {
		auto season_id = std::string{};
		if (!assign_if_match(season_id, word, season_id_matcher)) { return false; }

		KLIB_ASSERT(season_id.size() == std::string_view{"S00"}.size());
		auto const season = std::string_view{season_id}.substr(1);
		m_ret.season_id = SeasonId{.number = util::to_int(season)};
		return true;
	}

	auto is_year(std::string_view const word) -> bool { return assign_if_match(m_ret.year, word, year_matcher); }
	auto is_resolution(std::string_view const word) -> bool { return assign_if_match(m_ret.resolution, word, resolution_matcher); }

	void join_word(std::string_view const word) {
		if (m_skip_titles) { return; }
		if (word == "-") { return; }

		if (!m_ret.title.empty() && m_ret.episode_id) {
			util::join_to(m_ret.subtitle, word);
		} else {
			util::join_to(m_ret.title, word);
		}
	}

	MediaFile::Metadata m_ret{};
	bool m_skip_titles{};
	bool m_next_is_season{};
};
} // namespace

auto MediaFile::Metadata::build(std::string filename) -> Metadata { return MetadataBuilder{}(std::move(filename)); }

auto MediaFile::Metadata::to_table() const -> std::string {
	auto table = klib::TextTable::Builder{}.add_column("key").add_column("value").build();
	auto row = std::vector<std::string>{};
	row = {"title", title};
	table.push_row(std::move(row));
	row = {"subtitle", subtitle};
	table.push_row(std::move(row));
	row = {"season", season_id ? season_id->format() : ""};
	table.push_row(std::move(row));
	row = {"episode", episode_id ? episode_id->format() : ""};
	table.push_row(std::move(row));
	row = {"year", year};
	table.push_row(std::move(row));
	row = {"resolution", resolution};
	table.push_row(std::move(row));
	return table.serialize();
}
} // namespace vifo
