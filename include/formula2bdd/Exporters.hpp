#pragma once

#include <string>

#include "formula2bdd/BddManager.hpp"

namespace f2b {

// Graphviz DOT. Solid edge = high/1, dashed edge = low/0.
// Terminals are drawn as boxes (False / True).
std::string toDot(const BddManager& mgr, NodeId root, const std::string& title = "");

// Machine-readable JSON describing the reachable nodes and edges.
std::string toJson(const BddManager& mgr, NodeId root);

} // namespace f2b
