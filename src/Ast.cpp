#include "formula2bdd/Ast.hpp"

namespace f2b {

void collectVars(const AstNode* node, std::vector<std::string>& order,
                 std::set<std::string>& seen) {
    if (!node) return;
    if (node->kind == AstKind::Var) {
        if (seen.insert(node->var).second) order.push_back(node->var);
        return;
    }
    collectVars(node->left.get(), order, seen);
    collectVars(node->right.get(), order, seen);
}

std::vector<std::string> variablesOf(const AstNode* root) {
    std::vector<std::string> order;
    std::set<std::string> seen;
    collectVars(root, order, seen);
    return order;
}

} // namespace f2b
