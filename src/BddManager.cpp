#include "formula2bdd/BddManager.hpp"

namespace f2b {

BddManager::BddManager(std::vector<std::string> order) : order_(std::move(order)) {
    // Reserve ids 0 and 1 for the terminals. Their fields are never read.
    nodes_.push_back(BddNode{-1, kFalse, kFalse}); // id 0: False
    nodes_.push_back(BddNode{-1, kTrue, kTrue});   // id 1: True
}

NodeId BddManager::makeNode(int varIndex, NodeId low, NodeId high) {
    // Rule 1: redundant-node elimination. If both edges lead to the same
    // place, this variable is irrelevant here -> skip the node entirely.
    if (low == high) return low;

    // Rule 2: isomorphic-subgraph merging. Reuse an identical node if present.
    auto key = std::make_tuple(varIndex, low, high);
    auto it = unique_.find(key);
    if (it != unique_.end()) return it->second;

    NodeId id = static_cast<NodeId>(nodes_.size());
    nodes_.push_back(BddNode{varIndex, low, high});
    unique_.emplace(key, id);
    return id;
}

NodeId BddManager::buildRec(const AstNode* root, int level, Assignment& a) {
    // All variables assigned: the formula is now ground; evaluate to a terminal.
    if (level == static_cast<int>(order_.size())) {
        return evaluate(root, a) ? kTrue : kFalse;
    }

    const std::string& var = order_[level];

    a[var] = false;
    NodeId low = buildRec(root, level + 1, a);
    a[var] = true;
    NodeId high = buildRec(root, level + 1, a);

    return makeNode(level, low, high);
}

NodeId BddManager::build(const AstNode* root) {
    Assignment a;
    return buildRec(root, 0, a);
}

bool BddManager::evalPath(NodeId cur, const Assignment& a,
                          std::vector<NodeId>* trace) const {
    while (!isTerminal(cur)) {
        if (trace) trace->push_back(cur);
        const BddNode& n = nodes_[cur];
        auto it = a.find(order_[n.varIndex]);
        bool v = (it != a.end()) && it->second;
        cur = v ? n.high : n.low;
    }
    if (trace) trace->push_back(cur);
    return cur == kTrue;
}

void BddManager::collectLive(NodeId id, std::vector<char>& seen) const {
    if (isTerminal(id) || seen[id]) return;
    seen[id] = 1;
    collectLive(nodes_[id].low, seen);
    collectLive(nodes_[id].high, seen);
}

size_t BddManager::liveNodeCount(NodeId root) const {
    std::vector<char> seen(nodes_.size(), 0);
    collectLive(root, seen);
    size_t count = 0;
    for (size_t i = 2; i < seen.size(); ++i) count += seen[i] ? 1 : 0;
    return count;
}

} // namespace f2b
