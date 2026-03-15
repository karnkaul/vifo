#include "klib/text_table.hpp"
#include "vifo/directory_renamer.hpp"
#include "vifo/exit_code.hpp"
#include <cstdlib>
#include <exception>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

namespace vifo::cli {
namespace {
auto test(std::string_view const arg1, std::string_view const arg2) -> ExitCode {
	{
		auto const input_expression = std::string_view{"{year} - {title}"};
		auto const output_expression = std::string_view{"({year}) {title}"};

		auto renamer = DirectoryRenamer::create(input_expression, output_expression);
		if (!renamer) {
			std::println(stderr, "{}", renamer.error().message);
			return ExitCode::Failure;
		}

		auto manifest = renamer->build_manifest(arg1);
		std::println("manifest root: {}", manifest.root.generic_string());
		auto table = klib::TextTable::Builder{}.add_column("#", klib::TextTable::Align::Right).add_column("destination").add_column("source").build();
		// std::println("manifest:\n root\t: {}\n entries\t:\n", manifest.root.generic_string());
		auto row = std::vector<std::string>{};
		row.reserve(3);
		for (auto const [index, entry] : std::views::enumerate(manifest.entries)) {
			// std::println("{}. {} >> {}", index + 1, entry.source.generic_string(), entry.destination.generic_string());
			row.push_back(std::format("{}", index + 1));
			row.push_back(entry.destination.generic_string());
			row.push_back(entry.source.generic_string());
			table.push_row(std::move(row));
		}
		std::println("{}", table.serialize());

		return ExitCode::Success;
	}
	{
		auto const input_expression = std::string_view{"{year} - {title}"};
		auto const output_expression = std::string_view{"({year}) {title}"};

		auto interpolator_result = create_interpolator(input_expression, output_expression);
		if (!interpolator_result) {
			std::println(stderr, "{}", interpolator_result.error().message);
			return ExitCode::Failure;
		}

		auto interpolator = std::move(*interpolator_result);
		auto input_string = std::string_view{"1942 - Boomsie"};
		auto output_string = interpolator->format(input_string);
		std::println("input\t: {}", input_string);
		if (output_string.empty()) {
			std::println("not a match");
		} else {
			std::println("output\t: {}", output_string);
		}
		return ExitCode::Success;
	}
}
} // namespace
} // namespace vifo::cli

auto main(int argc, char** argv) -> int {
	try {
		// return vifo::cli::App{}.run(argc, argv);
		if (argc < 3) { return EXIT_FAILURE; }
		return int(vifo::cli::test(argv[1], argv[2]));
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println(stderr, "PANIC!");
		return EXIT_FAILURE;
	}
}
