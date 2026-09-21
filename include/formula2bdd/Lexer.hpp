#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "formula2bdd/Token.hpp"

namespace f2b {

// Thrown when the lexer encounters a byte sequence it cannot turn into a token.
class LexError : public std::runtime_error {
public:
    explicit LexError(const std::string& msg) : std::runtime_error(msg) {}
};

// Turns a UTF-8 formula string into a flat list of tokens. Supports both the
// Unicode operators (¬ ∧ ∨ ⊕ → ↔) and ASCII aliases (~ ! & | ^ -> <->).
class Lexer {
public:
    explicit Lexer(std::string source);

    // Produces the full token stream, terminated by a single End token.
    std::vector<Token> tokenize();

private:
    std::string src_;
    size_t pos_ = 0;

    bool atEnd() const { return pos_ >= src_.size(); }
    char peek() const { return src_[pos_]; }

    // Returns true (and advances) if the raw bytes at the cursor equal `s`.
    bool consumeLiteral(const std::string& s);
    Token lexIdentifierOrConst();
};

} // namespace f2b
