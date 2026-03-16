#include "vifo/util/progress.hpp"
#include <print>

namespace vifo::util {
void OverText::print_line(std::string line) {
	if (line.empty()) { return; }

	if (!m_current.empty()) {
		for (auto& c : m_current) { c = ' '; }
		std::print("\r{}\r", m_current);
		std::fflush(stdout);
	}

	m_current = std::move(line);
	std::print("{}", m_current);
	std::fflush(stdout);
}

void OverText::finish() {
	m_current.clear();
	std::println();
}

Progress::Progress(std::int64_t const total) : m_total(total) { print(); }

void Progress::set_completed(std::int64_t const count) {
	if (count == m_completed) { return; }
	m_completed = count;
	print();
}

void Progress::finish() {
	print();
	m_text.finish();
}

void Progress::print() { m_text.print_line(std::format("{} [{}/{}]", title, m_completed, m_total)); }
} // namespace vifo::util
