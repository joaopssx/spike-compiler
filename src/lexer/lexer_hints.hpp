#ifndef SPIKE_LEXER_HINTS_HPP
#define SPIKE_LEXER_HINTS_HPP

namespace spike::lexer_hints {

// Codigos e dicas - mantidos perto um do outro pra ser facil de auditar.
inline constexpr const char* kE001Hint =
    "Portugol não usa este caractere. Verifique se não é um erro de digitação.";
inline constexpr const char* kE002Hint =
    "Feche a string com aspas duplas antes do fim da linha.";
inline constexpr const char* kE003Hint =
    "Números decimais precisam ter dígito após o ponto: use 3.0 em vez de 3.";
inline constexpr const char* kE004Hint =
    "Feche o comentário com } antes do fim do arquivo.";
inline constexpr const char* kE005Hint = "salve o arquivo como UTF-8.";
inline constexpr const char* kNotEqualHint = "para 'diferente' use '<>': se x <> 0 entao.";
inline constexpr const char* kNotHint = "para negar use a palavra 'nao': se nao achou entao.";
inline constexpr const char* kAndHint = "em Portugol o 'e' lógico é a palavra 'e': x > 0 e x < 10.";
inline constexpr const char* kOrHint = "em Portugol o 'ou' lógico é a palavra 'ou': x < 0 ou x > 10.";
inline constexpr const char* kSemicolonHint = "Portugol não usa ';' no fim da linha: apague o ';'.";

} // namespace spike::lexer_hints

#endif // SPIKE_LEXER_HINTS_HPP
