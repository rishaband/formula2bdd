#pragma once

#include <map>
#include <string>

#include "formula2bdd/Ast.hpp"

namespace f2b {

using Assignment = std::map<std::string, bool>;

// Directly evaluates the AST under a full truth assignment.
// This is the ground-truth oracle used to validate the BDD.
bool evaluate(const AstNode* node, const Assignment& assignment);

} // namespace f2b
