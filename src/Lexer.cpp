#include "formula2bdd/Lexer.hpp"

#include <cctype>

namespace f2b {

namespace {
// UTF-8 byte sequences for the Unicode operator glyphs.
const std::string kNot = "\u00ac"; // ¬
const std::string kAnd = "\u2227"; // ∧
const std::string kOr  = "\u2228"; // ∨
const std::string kXor = "\u2295"; // ⊕
const std::string kImp = "\u2192"; // →
const std::string kIff = "\u2194"; // ↔

bool isIdentStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}
bool isIdentPart(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}
} // namespace

Lexer::Lexer(std::string source) : src_(std::move(source)) {}

bool Lexer::consumeLiteral(const std::string& s) {
    if (src_.compare(pos_, s.size(), s) == 0) {
        pos_ += s.size();
        return true;
    }
    return false;
}

Token Lexer::lexIdentifierOrConst() {
    size_t start = pos_;
    while (!atEnd() && isIdentPart(peek())) pos_++;
    std::string text = src_.substr(start, pos_ - start);
    return Token{TokenType::Var, text, false, start};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> out;
    while (!atEnd()) {
        char c = peek();

        // Whitespace.
        if (std::isspace(static_cast<unsigned char>(c))) {
            pos_++;
            continue;
        }

        size_t start = pos_;

        // Constants 0 / 1.
        if (c == '0' || c == '1') {
            pos_++;
            out.push_back(Token{TokenType::Const, std::string(1, c), c == '1', start});
            continue;
        }

        // Parentheses.
        if (c == '(') { pos_++; out.push_back(Token{TokenType::LParen, "(", false, start}); continue; }
        if (c == ')') { pos_++; out.push_back(Token{TokenType::RParen, ")", false, start}); continue; }

        // ASCII single-char operators.
        if (c == '~' || c == '!') { pos_++; out.push_back(Token{TokenType::Not, std::string(1, c), false, start}); continue; }
        if (c == '&') { pos_++; out.push_back(Token{TokenType::And, "&", false, start}); continue; }
        if (c == '|') { pos_++; out.push_back(Token{TokenType::Or, "|", false, start}); continue; }
        if (c == '^') { pos_++; out.push_back(Token{TokenType::Xor, "^", false, start}); continue; }

        // Multi-char ASCII: <-> before ->.
        if (consumeLiteral("<->")) { out.push_back(Token{TokenType::Iff, "<->", false, start}); continue; }
        if (consumeLiteral("->"))  { out.push_back(Token{TokenType::Imp, "->", false, start}); continue; }

        // Unicode operators (checked as raw byte sequences).
        if (consumeLiteral(kNot)) { out.push_back(Token{TokenType::Not, kNot, false, start}); continue; }
        if (consumeLiteral(kAnd)) { out.push_back(Token{TokenType::And, kAnd, false, start}); continue; }
        if (consumeLiteral(kOr))  { out.push_back(Token{TokenType::Or,  kOr,  false, start}); continue; }
        if (consumeLiteral(kXor)) { out.push_back(Token{TokenType::Xor, kXor, false, start}); continue; }
        if (consumeLiteral(kImp)) { out.push_back(Token{TokenType::Imp, kImp, false, start}); continue; }
        if (consumeLiteral(kIff)) { out.push_back(Token{TokenType::Iff, kIff, false, start}); continue; }

        // Identifiers (variables).
        if (isIdentStart(c)) { out.push_back(lexIdentifierOrConst()); continue; }

        throw LexError("Unexpected character at byte " + std::to_string(pos_) +
                       ": '" + std::string(1, c) + "'");
    }
    out.push_back(Token{TokenType::End, "", false, pos_});
    return out;
}

} // namespace f2b
