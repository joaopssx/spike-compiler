#include "codegen/codegen_internal.hpp"
#include "spike/runtime_text.hpp"
#include "spike/symbol_table.hpp"

#include <string>
#include <utility>

namespace spike {

namespace codegen_detail {

void CWriter::line(const std::string& text) {
    out_.append(static_cast<std::size_t>(depth_) * 4, ' ');
    out_ += text;
    out_ += '\n';
}

void CWriter::blank() {
    out_ += '\n';
}

void CWriter::raw(const std::string& text) {
    out_ += text;
}

void CWriter::indent() {
    ++depth_;
}

void CWriter::dedent() {
    if (depth_ > 0) --depth_;
}

const std::string& CWriter::str() const {
    return out_;
}

Generator::Generator(const Program& program, const TypeMap& types, const ConstantMap& constants,
                     std::string source_name)
    : program_(program),
      types_(types),
      constants_(constants),
      source_name_(std::move(source_name)) {
    for (const VarDecl& decl : program_.globals) {
        var_types_.emplace(symbol_key(decl.name), decl.type);
    }
    collect_array_dims();
    for (const ProcedureDecl& proc : program_.procedures) {
        procedures_.emplace(symbol_key(proc.name), &proc);
    }
}

CodegenResult Generator::run() {
    emit_header();
    emit_constants();
    emit_globals();
    emit_prototypes();
    emit_procedures();
    emit_main();
    if (failed()) {
        result_.c_source.clear();
        return result_;
    }
    result_.ok = true;
    result_.c_source = out_.str();
    return result_;
}

bool Generator::failed() const {
    return !result_.error_code.empty();
}

void Generator::fail(const char* code, const std::string& message, int line, int col) {
    if (failed()) return;
    result_.error_code = code;
    result_.error_message = message;
    result_.error_line = line;
    result_.error_col = col;
}

void Generator::emit_header() {
    out_.line("/* Gerado pelo Spike " SPIKE_VERSION " a partir de " +
              comment_safe(source_name_) + " */");
    out_.blank();
    std::string runtime(runtime_source(), runtime_source_size());
    out_.raw(runtime);
    if (!runtime.empty() && runtime.back() != '\n') out_.blank();
    out_.blank();
}

void Generator::emit_constants() {
    if (program_.consts.empty()) return;
    for (const ConstDecl& decl : program_.consts) {
        const auto it = constants_.find(symbol_key(decl.name));
        const char* type = it == constants_.end() ? nullptr : c_type(it->second.type);
        if (!type) {
            fail("E900", "erro interno do gerador: constante '" + decl.name + "' sem valor",
                 decl.line, decl.col);
            return;
        }
        out_.line(std::string("const ") + type + " " + c_name(decl.name) + " = " +
                  constant_literal(it->second) + ";");
    }
    out_.blank();
}

void Generator::emit_globals() {
    if (program_.globals.empty()) return;
    for (const VarDecl& decl : program_.globals) {
        const char* type = c_type(decl.type);
        if (!type) {
            fail("E900", "erro interno do gerador: variável '" + decl.name + "' sem tipo",
                 decl.line, decl.col);
            return;
        }
        if (decl.dims.empty()) {
            out_.line(std::string(type) + " " + c_name(decl.name) + " = " +
                      default_value(decl.type) + ";");
        } else {
            out_.line(std::string(type) + " " + c_name(decl.name) + "[" +
                      std::to_string(element_count(decl.name)) + "];");
        }
    }
    out_.blank();
}

void Generator::emit_main() {
    out_.line("int main(void) {");
    out_.indent();
    out_.line("spike_init();");
    for (const VarDecl& decl : program_.globals) {
        if (!decl.dims.empty() && decl.type == TypeKind::String) {
            out_.line("spike_fill_str(" + c_name(decl.name) + ", " +
                      std::to_string(element_count(decl.name)) + ");");
        }
    }
    emit_block(program_.body);
    out_.line("spike_flush();");
    out_.line("spike_cleanup();");
    out_.line("return 0;");
    out_.dedent();
    out_.line("}");
}

} // namespace codegen_detail

CodegenResult generate_c(const Program& program,
                         const std::unordered_map<const Expr*, TypeKind>& expr_types,
                         const ConstantMap& constants, const std::string& source_name) {
    codegen_detail::Generator generator(program, expr_types, constants, source_name);
    return generator.run();
}

} // namespace spike
