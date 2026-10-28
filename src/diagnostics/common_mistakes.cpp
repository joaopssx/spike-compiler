#include "spike/suggest.hpp"

#include <cctype>
#include <string>
#include <unordered_map>

namespace spike {

namespace {

const std::unordered_map<std::string, std::string>& mistakes() {
    static const std::string write = "em Portugol use escreva() ou escreval().";
    static const std::string read = "em Portugol use leia(): leia(nome).";
    static const std::string function =
        "em Portugol use 'funcao nome(...): tipo' ou 'procedimento nome(...)'.";
    static const std::string integer = "em Portugol o tipo de número inteiro é 'inteiro'.";
    static const std::string real = "em Portugol o tipo de número com vírgula é 'real'.";
    static const std::string text = "em Portugol o tipo de texto é 'caractere'.";
    static const std::string logical = "em Portugol o tipo verdadeiro/falso é 'logico'.";
    static const std::string boolean = "em Portugol use verdadeiro e falso.";
    static const std::unordered_map<std::string, std::string> table = {
        {"print", write},     {"printf", write},     {"cout", write},
        {"puts", write},      {"println", write},    {"input", read},
        {"scanf", read},      {"cin", read},         {"readline", read},
        {"if", "em Portugol use 'se ... entao ... fimse'."},
        {"else", "em Portugol use 'senao'."},
        {"while", "em Portugol use 'enquanto ... faca ... fimenquanto'."},
        {"for", "em Portugol use 'para i de 1 ate 10 faca ... fimpara'."},
        {"do", "em Portugol use 'repita ... ate condição'."},
        {"switch", "em Portugol use 'escolha ... caso ... fimescolha'."},
        {"break", "em Portugol use 'interrompa'."},
        {"return", "em Portugol use 'retorne'."},
        {"function", function}, {"def", function},   {"void", function},
        {"int", integer},     {"integer", integer},  {"long", integer},
        {"float", real},      {"double", real},      {"string", text},
        {"str", text},        {"char", text},        {"text", text},
        {"texto", text},      {"bool", logical},     {"boolean", logical},
        {"true", boolean},    {"false", boolean},
        {"and", "em Portugol o 'e' lógico é a palavra 'e': x > 0 e x < 10."},
        {"or", "em Portugol o 'ou' lógico é a palavra 'ou'."},
        {"not", "em Portugol a negação é a palavra 'nao'."},
    };
    return table;
}

} // namespace

std::optional<std::string> common_mistake(const std::string& word) {
    std::string key;
    for (unsigned char c : word) key += static_cast<char>(std::tolower(c));
    const auto it = mistakes().find(key);
    if (it == mistakes().end()) return std::nullopt;
    return it->second;
}

} // namespace spike
