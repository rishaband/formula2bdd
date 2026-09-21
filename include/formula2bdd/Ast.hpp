#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace f2b {

enum class AstKind { Var, Const, Not, And, Or, Xor, Imp, Iff };

struct AstNode {
    AstKind kind;
    std::string var;         // valid when kind == Var
    bool constValue = false; // valid when kind == Const
    std::unique_ptr<AstNode> left;
    std::unique_ptr<AstNode> right; // null for Not / Var / Const

    explicit AstNode(AstKind k) : kind(k) {}
};

using AstPtr = std::unique_ptr<AstNode>;

// Convenience constructors.
inline AstPtr makeVar(std::string name) {
    auto n = std::make_unique<AstNode>(AstKind::Var);
    n->var = std::move(name);
    return n;
}
inline AstPtr makeConst(bool v) {
    auto n = std::make_unique<AstNode>(AstKind::Const);
    n->constValue = v;
    return n;
}
inline AstPtr makeNot(AstPtr child) {
    auto n = std::make_unique<AstNode>(AstKind::Not);
    n->left = std::move(child);
    return n;
}
inline AstPtr makeBinary(AstKind k, AstPtr l, AstPtr r) {
    auto n = std::make_unique<AstNode>(k);
    n->left = std::move(l);
    n->right = std::move(r);
    return n;
}

// Collects variable names in order of first appearance (left-to-right).
void collectVars(const AstNode* node, std::vector<std::string>& order,
                 std::set<std::string>& seen);
std::vector<std::string> variablesOf(const AstNode* root);

} // namespace f2b
