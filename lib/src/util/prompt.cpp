#include "vifo/util/prompt.hpp"
#include "klib/assert.hpp"
#include "vifo/util/util.hpp"
#include <iostream>
#include <print>
#include <string_view>

namespace vifo {
using prompt::Option;
using prompt::Selection;

auto util::prompt_line(std::string_view const message, std::move_only_function<bool(std::string)> pred) -> Selection {
	static constexpr auto max_attempts_v{3};
	auto line = std::string{};
	for (auto attempt = 0; attempt < max_attempts_v; ++attempt) {
		std::print("\n{}\n> ", message);
		std::getline(std::cin, line);
		std::println();
		if (pred(std::move(line))) { return Selection::Line; }
		std::println(stderr, "invalid input");
	}
	return Selection::Invalid;
}

auto util::prompt_confirm(std::string_view const message) -> Selection {
	auto const msg = std::format("{} (y/N):", message);
	auto input = std::string{};
	auto const pred = [&](std::string in) {
		if (in.empty() || in == "n" || in == "y") {
			input = std::move(in);
			return true;
		}
		return false;
	};
	auto const selection = prompt_line(msg, pred);
	if (selection == Selection::Invalid) { return selection; }

	KLIB_ASSERT(selection == Selection::Line);
	if (input == "y") { return Selection::Confirm; }
	return Selection::Exit;
}

auto util::prompt_options(std::span<Option const> options, bool const empty_is_exit) -> Selection {
	auto message = std::string{};
	auto number = 0;
	for (auto const& option : options) { std::format_to(std::back_inserter(message), "{}) {}\n", ++number, option.text); }
	message += "q) quit";

	auto const pred = [&](std::string_view input) {
		if (input.empty()) {
			if (!empty_is_exit) { return false; }
			input = "q";
		}
		if (input == "q") {
			number = -1;
			return true;
		}
		number = util::to_int(input);
		return number >= 1 && number <= int(options.size());
	};

	auto const selection = prompt_line(message, pred);
	if (selection == Selection::Invalid) { return selection; }

	KLIB_ASSERT(selection == Selection::Line);
	if (number == -1) { return Selection::Exit; }
	KLIB_ASSERT(number > 0);
	auto const index = std::size_t(number - 1);
	auto const& option = options[index];
	if (option.callback) { option.callback(); }
	return Selection::Option;
}
} // namespace vifo
