#include "vifo/transaction.hpp"

namespace vifo {
void Transaction::triage_record(Record record, Outcome const outcome) {
	switch (outcome) {
	case Outcome::Success: success.push_back(std::move(record)); break;
	case Outcome::Failure: failure.push_back(std::move(record)); break;
	default:
	case Outcome::Pass: pass.push_back(std::move(record)); break;
	}
}
} // namespace vifo
