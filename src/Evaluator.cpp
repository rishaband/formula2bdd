#include "formula2bdd/Evaluator.hpp"

#include <stdexcept>

namespace f2b {

bool evaluate(const AstNode* node, const Assignment& assignment) {
    switch (node->kind) {
        case AstKind::Const:
            return node->constValue;
        case AstKind::Var: {
            auto it = assignment.find(node->var);
            if (it == assignment.end())
                throw std::runtime_error("Unassigned variable: " + node->var);
            return it->second;
        }
        case AstKind::Not:
            return !evaluate(node->left.get(), assignment);
        case AstKind::And:
            return evaluate(node->left.get(), assignment) &&
                   evaluate(node->right.get(), assignment);
        case AstKind::Or:
            return evaluate(node->left.get(), assignment) ||
                   evaluate(node->right.get(), assignment);
        case AstKind::Xor:
            return evaluate(node->left.get(), assignment) !=
                   evaluate(node->right.get(), assignment);
        case AstKind::Imp:
            return !evaluate(node->left.get(), assignment) ||
                   evaluate(node->right.get(), assignment);
        case AstKind::Iff:
            return evaluate(node->left.get(), assignment) ==
                   evaluate(node->right.get(), assignment);
    }
    throw std::runtime_error("Unknown AST node kind");
}

} // namespace f2b
