#pragma once
#include <cstdint>
#include <string>

namespace vifo::util {
class OverText {
  public:
	void print_line(std::string line);
	void finish();

  private:
	std::string m_current{};
};

class Progress {
  public:
	explicit Progress(std::int64_t total);

	void increment_completed() { set_completed(m_completed + 1); }
	void set_completed(std::int64_t count);
	void finish();

	std::string title{"progress:"};

  private:
	void print();

	OverText m_text{};
	std::int64_t m_total{};
	std::int64_t m_completed{};
};
} // namespace vifo::util
