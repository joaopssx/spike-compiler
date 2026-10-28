#ifndef SPIKE_SUMMARY_HPP
#define SPIKE_SUMMARY_HPP

#include <string>

namespace spike {

// "falhou: 2 erros e 1 aviso" (no warning part when there are none).
std::string failure_summary(int errors, int warnings);

// "0 erros, 1 aviso": appended to the "ok:" line when there are warnings.
std::string success_counts(int errors, int warnings);

// "Para entender um erro: spike --explain E400".
std::string explain_suggestion(const std::string& code);

} // namespace spike

#endif // SPIKE_SUMMARY_HPP
