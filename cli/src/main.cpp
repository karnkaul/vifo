#include "log.hpp"
#include "vifo/environment.hpp"
#include "vifo/exit_code.hpp"
#include <cstdlib>
#include <exception>
#include <print>
#include <string_view>

namespace vifo::cli {
namespace {
auto test(std::string_view const arg1, std::string_view const arg2) -> ExitCode {
	{
		auto const input_format = std::string_view{"{year} - {title}"};
		auto env = Environment::create(input_format);
		if (!env) {
			log.error("{}", env.error().message);
			return ExitCode::Failure;
		}

		auto expr = std::string_view{"2015 - ABC Murders"};
		auto fmt = std::string_view{"({year}) {title}"};
		auto text = env->interpolate(expr, fmt);
		std::println("expression\t: {}", expr);
		if (!text) {
			std::println(stderr, "error: {}", text.error().message);
			return ExitCode::Failure;
		}
		std::println("transform\t: {}", *text);

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
