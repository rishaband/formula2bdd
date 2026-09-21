#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "formula2bdd/Ast.hpp"
#include "formula2bdd/Evaluator.hpp"

namespace f2b {

// A BDD node is identified by an integer id.
//   id 0 -> terminal False
//   id 1 -> terminal True
//   id >=2 -> decision node stored in nodes_[id]
using NodeId = int;

constexpr NodeId kFalse = 0;
constexpr NodeId kTrue = 1;

struct BddNode {
    int varIndex;  // index into the variable order
    NodeId low;    // edge taken when the variable is False (0)
    NodeId high;   // edge taken when the variable is True (1)
};

// Builds and stores a Reduced Ordered BDD via Shannon expansion.
// Hash-consing through the unique table enforces the two reduction rules,
// giving a canonical representation for the chosen variable order.
class BddManager {
public:
    // `order` fixes the variable ordering used for every node.
    explicit BddManager(std::vector<std::string> order);

    // Builds the ROBDD for `root` and returns the id of its root node.
    NodeId build(const AstNode* root);

    // Follows the decision path for a full assignment down to a terminal.
    // `trace` (if non-null) receives the ids visited along the way.
    bool evalPath(NodeId root, const Assignment& a,
                  std::vector<NodeId>* trace = nullptr) const;

    bool isTerminal(NodeId id) const { return id == kFalse || id == kTrue; }
    const BddNode& node(NodeId id) const { return nodes_[id]; }
    const std::vector<std::string>& order() const { return order_; }
    const std::string& varName(int index) const { return order_[index]; }

    // Number of decision nodes actually reachable from `root`.
    size_t liveNodeCount(NodeId root) const;
    // Total decision nodes ever created (before dedup this would be larger).
    size_t tableSize() const { return nodes_.size() >= 2 ? nodes_.size() - 2 : 0; }

private:
    std::vector<std::string> order_;
    std::vector<BddNode> nodes_;                       // index 0,1 unused terminals
    std::map<std::tuple<int, NodeId, NodeId>, NodeId> unique_;

    // Reduction + hash-consing: the only way decision nodes are created.
    NodeId makeNode(int varIndex, NodeId low, NodeId high);

    // Shannon expansion carrying a partial assignment.
    NodeId buildRec(const AstNode* root, int level, Assignment& a);

    void collectLive(NodeId id, std::vector<char>& seen) const;
};

} // namespace f2b
