#include "klib/visitor.hpp"
#include "log.hpp"
#include "vifo/environment.hpp"
#include "vifo/exit_code.hpp"
#include "vifo/expression.hpp"
#include "vifo/interpolator.hpp"
#include <cstdlib>
#include <exception>
#include <print>
#include <string>
#include <string_view>

namespace vifo::cli {
namespace {
auto test(std::string_view const arg1, std::string_view const arg2) -> ExitCode {
	{
		auto const input_format = std::string_view{"{year} - {title}"};

		auto input_expression = expression::parse(input_format);
		if (!input_expression) {
			std::println(stderr, "{}", input_expression.error().message);
			return ExitCode::Failure;
		}

		auto const output_format = std::string_view{"({year}) {title}"};
		auto output_expression = expression::parse(output_format);
		if (!output_expression) {
			std::println(stderr, "{}", output_expression.error().message);
			return ExitCode::Failure;
		}

		auto interpolator_result = IInterpolator::create(std::move(*input_expression), std::move(*output_expression));
		if (!interpolator_result) {
			std::println(stderr, "{}", interpolator_result.error().message);
			return ExitCode::Failure;
		}

		auto interpolator = std::move(*interpolator_result);
		auto input_string = std::string_view{"1942 - Boomsie"};
		auto output_string = std::string{};
		auto const result = interpolator->format(input_string, output_string);
		std::println("input\t: {}", input_string);
		if (!result) {
			std::println("not a match");
		} else {
			std::println("output\t: {}", output_string);
		}
		return ExitCode::Success;
	}
	{
		auto const input_format = std::string_view{"{year:4} - {title:0}"};

		auto const expression = expression::parse(input_format);
		if (!expression) {
			std::println(stderr, "{}", expression.error().message);
			return ExitCode::Failure;
		}

		static constexpr auto input_v = std::string_view{"1942 - Boomsie"};
		auto const visitor = klib::Visitor{
			[&](expression::Substring const& s) { std::println("[substr] '{}'", s.text); },
			[&](expression::Identifier const& i) { std::println("[identifier] name: '{}', length = {}", i.name, i.length); },
		};
		for (auto const& subexpr : expression->atoms) { std::visit(visitor, subexpr); }
	}
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
