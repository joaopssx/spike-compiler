#include "spike/summary.hpp"

namespace spike {

namespace {

std::string counted(int count, const char* singular, const char* plural) {
    return std::to_string(count) + " " + (count == 1 ? singular : plural);
}

} // namespace

std::string failure_summary(int errors, int warnings) {
    std::string text = "falhou: " + counted(errors, "erro", "erros");
    if (warnings > 0) text += " e " + counted(warnings, "aviso", "avisos");
    return text;
}

std::string success_counts(int errors, int warnings) {
    return counted(errors, "erro", "erros") + ", " + counted(warnings, "aviso", "avisos");
}

std::string explain_suggestion(const std::string& code) {
    return "Para entender um erro: spike --explain " + code;
}

} // namespace spike
