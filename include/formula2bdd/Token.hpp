#pragma once

#include <string>

namespace f2b {

enum class TokenType {
    Var,     // identifier, e.g. p, q, r
    Const,   // literal 0 or 1
    Not,     // ¬  ~  !
    And,     // ∧  &
    Or,      // ∨  |
    Xor,     // ⊕  ^
    Imp,     // →  ->
    Iff,     // ↔  <->
    LParen,  // (
    RParen,  // )
    End      // end of input
};

struct Token {
    TokenType type;
    std::string text;   // original spelling (variable name, operator, etc.)
    bool value = false; // for Const: the literal truth value
    size_t pos = 0;     // byte offset in the source, for error messages
};

} // namespace f2b
