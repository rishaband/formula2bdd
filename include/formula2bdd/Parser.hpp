#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "formula2bdd/Ast.hpp"
#include "formula2bdd/Token.hpp"

namespace f2b {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

// Recursive-descent parser. Precedence, tightest to loosest:
//   ¬  >  ∧  >  ∨  >  ⊕  >  →  >  ↔
// → is right-associative; the others are left-associative.
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    AstPtr parse();

private:
    std::vector<Token> toks_;
    size_t pos_ = 0;

    const Token& peek() const { return toks_[pos_]; }
    const Token& advance() { return toks_[pos_++]; }
    bool check(TokenType t) const { return peek().type == t; }
    bool match(TokenType t);
    void expect(TokenType t, const char* what);

    AstPtr parseIff();
    AstPtr parseImp();
    AstPtr parseXor();
    AstPtr parseOr();
    AstPtr parseAnd();
    AstPtr parseNot();
    AstPtr parseAtom();
};

// Convenience: lex + parse a formula string into an AST.
AstPtr parseFormula(const std::string& source);

} // namespace f2b
