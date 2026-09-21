#include "formula2bdd/Parser.hpp"

#include "formula2bdd/Lexer.hpp"

namespace f2b {

Parser::Parser(std::vector<Token> tokens) : toks_(std::move(tokens)) {}

bool Parser::match(TokenType t) {
    if (check(t)) { advance(); return true; }
    return false;
}

void Parser::expect(TokenType t, const char* what) {
    if (!check(t)) {
        throw ParseError(std::string("Expected ") + what + " at byte " +
                         std::to_string(peek().pos) + " but found '" +
                         (peek().type == TokenType::End ? "<end>" : peek().text) + "'");
    }
    advance();
}

AstPtr Parser::parse() {
    AstPtr root = parseIff();
    if (!check(TokenType::End)) {
        throw ParseError("Unexpected trailing input at byte " +
                         std::to_string(peek().pos) + ": '" + peek().text + "'");
    }
    return root;
}

// ↔ : lowest precedence, left-associative.
AstPtr Parser::parseIff() {
    AstPtr left = parseImp();
    while (match(TokenType::Iff)) {
        AstPtr right = parseImp();
        left = makeBinary(AstKind::Iff, std::move(left), std::move(right));
    }
    return left;
}

// → : right-associative.
AstPtr Parser::parseImp() {
    AstPtr left = parseXor();
    if (match(TokenType::Imp)) {
        AstPtr right = parseImp(); // recurse right for right-associativity
        return makeBinary(AstKind::Imp, std::move(left), std::move(right));
    }
    return left;
}

AstPtr Parser::parseXor() {
    AstPtr left = parseOr();
    while (match(TokenType::Xor)) {
        AstPtr right = parseOr();
        left = makeBinary(AstKind::Xor, std::move(left), std::move(right));
    }
    return left;
}

AstPtr Parser::parseOr() {
    AstPtr left = parseAnd();
    while (match(TokenType::Or)) {
        AstPtr right = parseAnd();
        left = makeBinary(AstKind::Or, std::move(left), std::move(right));
    }
    return left;
}

AstPtr Parser::parseAnd() {
    AstPtr left = parseNot();
    while (match(TokenType::And)) {
        AstPtr right = parseNot();
        left = makeBinary(AstKind::And, std::move(left), std::move(right));
    }
    return left;
}

AstPtr Parser::parseNot() {
    if (match(TokenType::Not)) {
        return makeNot(parseNot());
    }
    return parseAtom();
}

AstPtr Parser::parseAtom() {
    if (check(TokenType::Var)) {
        return makeVar(advance().text);
    }
    if (check(TokenType::Const)) {
        return makeConst(advance().value);
    }
    if (match(TokenType::LParen)) {
        AstPtr inner = parseIff();
        expect(TokenType::RParen, "')'");
        return inner;
    }
    throw ParseError("Expected variable, constant, or '(' at byte " +
                     std::to_string(peek().pos) + " but found '" +
                     (peek().type == TokenType::End ? "<end>" : peek().text) + "'");
}

AstPtr parseFormula(const std::string& source) {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    return parser.parse();
}

} // namespace f2b
