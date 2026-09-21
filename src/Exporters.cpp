#include "formula2bdd/Exporters.hpp"

#include <sstream>
#include <vector>

namespace f2b {

namespace {
// Depth-first collection of the decision nodes reachable from `root`.
void reachable(const BddManager& mgr, NodeId id, std::vector<char>& seen,
               std::vector<NodeId>& order) {
    if (mgr.isTerminal(id) || seen[id]) return;
    seen[id] = 1;
    order.push_back(id);
    reachable(mgr, mgr.node(id).low, seen, order);
    reachable(mgr, mgr.node(id).high, seen, order);
}

std::vector<NodeId> reachableNodes(const BddManager& mgr, NodeId root) {
    std::vector<char> seen(mgr.tableSize() + 2, 0);
    std::vector<NodeId> order;
    reachable(mgr, root, seen, order);
    return order;
}
} // namespace

std::string toDot(const BddManager& mgr, NodeId root, const std::string& title) {
    std::ostringstream os;
    os << "digraph ROBDD {\n";
    os << "  labelloc=\"t\";\n";
    if (!title.empty()) os << "  label=\"" << title << "\";\n";
    os << "  node [fontname=\"Helvetica\"];\n\n";

    // Terminal case: the whole function is a constant.
    if (mgr.isTerminal(root)) {
        os << "  T" << root << " [shape=box, label=\""
           << (root == kTrue ? "True" : "False") << "\"];\n";
        os << "}\n";
        return os.str();
    }

    auto nodes = reachableNodes(mgr, root);

    // Which terminals are referenced?
    bool usesFalse = false, usesTrue = false;
    for (NodeId id : nodes) {
        const BddNode& n = mgr.node(id);
        for (NodeId c : {n.low, n.high}) {
            if (c == kFalse) usesFalse = true;
            if (c == kTrue) usesTrue = true;
        }
    }

    os << "  // decision nodes\n";
    for (NodeId id : nodes) {
        os << "  n" << id << " [shape=circle, label=\""
           << mgr.varName(mgr.node(id).varIndex) << "\"];\n";
    }
    os << "\n  // terminals\n";
    if (usesFalse) os << "  n0 [shape=box, label=\"False\"];\n";
    if (usesTrue)  os << "  n1 [shape=box, label=\"True\"];\n";

    os << "\n  // edges (solid = 1/high, dashed = 0/low)\n";
    for (NodeId id : nodes) {
        const BddNode& n = mgr.node(id);
        os << "  n" << id << " -> n" << n.low << " [style=dashed, label=\"0\"];\n";
        os << "  n" << id << " -> n" << n.high << " [style=solid, label=\"1\"];\n";
    }
    os << "}\n";
    return os.str();
}

std::string toJson(const BddManager& mgr, NodeId root) {
    std::ostringstream os;
    os << "{\n";
    os << "  \"root\": " << root << ",\n";

    // Variable order.
    os << "  \"order\": [";
    for (size_t i = 0; i < mgr.order().size(); ++i) {
        if (i) os << ", ";
        os << "\"" << mgr.order()[i] << "\"";
    }
    os << "],\n";

    os << "  \"terminals\": {\"false\": 0, \"true\": 1},\n";

    auto nodes = reachableNodes(mgr, root);
    os << "  \"nodes\": [\n";
    for (size_t i = 0; i < nodes.size(); ++i) {
        NodeId id = nodes[i];
        const BddNode& n = mgr.node(id);
        os << "    {\"id\": " << id
           << ", \"var\": \"" << mgr.varName(n.varIndex) << "\""
           << ", \"low\": " << n.low
           << ", \"high\": " << n.high << "}";
        os << (i + 1 < nodes.size() ? ",\n" : "\n");
    }
    os << "  ]\n";
    os << "}\n";
    return os.str();
}

} // namespace f2b
