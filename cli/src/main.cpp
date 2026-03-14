#include "log.hpp"
#include "vifo/binding.hpp"
#include "vifo/exit_code.hpp"
#include <cstdlib>
#include <exception>
#include <print>
#include <string_view>

namespace vifo::cli {
namespace {
auto test(std::string_view const arg1, std::string_view const arg2) -> ExitCode {
	auto const result = Binding::extract(arg1, arg2);
	if (!result) {
		log.error("{}", result.error().message);
		return ExitCode::Failure;
	}

	if (result->empty()) {
		log.info("no phrases found: fmt: '{}', text: '{}'", arg1, arg2);
		return ExitCode::Failure;
	}

	std::println("format\t: {}\ninput\t: {}", arg1, arg2);
	for (auto const& binding : *result) { std::println("{}: {}", binding.key, binding.value); }
	return ExitCode::Success;
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
