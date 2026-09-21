#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "formula2bdd/BddManager.hpp"
#include "formula2bdd/Evaluator.hpp"
#include "formula2bdd/Parser.hpp"

using namespace f2b;

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool cond, const std::string& what) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::cerr << "  FAIL: " << what << "\n";
    }
}

// Enumerate all 2^n assignments and confirm the BDD agrees with the direct
// evaluator on every one. This is the core correctness guarantee.
void checkAgainstOracle(const std::string& formula,
                        const std::vector<std::string>& order = {}) {
    AstPtr ast = parseFormula(formula);
    std::vector<std::string> vars = order.empty() ? variablesOf(ast.get()) : order;

    BddManager mgr(vars);
    NodeId root = mgr.build(ast.get());

    size_t n = vars.size();
    for (size_t mask = 0; mask < (size_t(1) << n); ++mask) {
        Assignment a;
        for (size_t i = 0; i < n; ++i) a[vars[i]] = (mask >> i) & 1;
        bool oracle = evaluate(ast.get(), a);
        bool viaBdd = mgr.evalPath(root, a);
        check(oracle == viaBdd,
              "formula '" + formula + "' mask " + std::to_string(mask));
    }
}

// Two logically-equivalent formulas must build the *identical* root id
// under the same variable order. That is the canonicity property.
void checkCanonical(const std::string& f1, const std::string& f2,
                    const std::vector<std::string>& order) {
    BddManager mgr(order);
    NodeId r1 = mgr.build(parseFormula(f1).get());
    NodeId r2 = mgr.build(parseFormula(f2).get());
    check(r1 == r2, "'" + f1 + "' and '" + f2 + "' should be canonical-equal");
}

} // namespace

int main() {
    std::cout << "Running Formula2BDD tests...\n";

    // Oracle agreement across a range of formulas / operators.
    checkAgainstOracle("p");
    checkAgainstOracle("~p");
    checkAgainstOracle("p & q");
    checkAgainstOracle("p | q");
    checkAgainstOracle("p -> q");
    checkAgainstOracle("p <-> q");
    checkAgainstOracle("p ^ q");
    checkAgainstOracle("(p | (q & r)) -> ~s");
    checkAgainstOracle("(p & q) | (~p & r)", {"p", "q", "r"});
    checkAgainstOracle("a <-> (b -> (c ^ d))");

    // Unicode operator forms should behave identically to ASCII.
    checkAgainstOracle("(p \u2228 (q \u2227 r)) \u2192 \u00acs");

    // Tautologies reduce to the True terminal; contradictions to False.
    {
        BddManager m1({"p"});
        check(m1.build(parseFormula("p | ~p").get()) == kTrue,
              "p | ~p reduces to True terminal");
        BddManager m2({"p"});
        check(m2.build(parseFormula("p & ~p").get()) == kFalse,
              "p & ~p reduces to False terminal");
    }

    // Redundant-node elimination: q never affects the result, so it must not
    // appear in the diagram (only p should remain -> a single decision node).
    {
        BddManager m({"p", "q"});
        NodeId r = m.build(parseFormula("p | (q & ~q)").get());
        check(m.liveNodeCount(r) == 1, "irrelevant variable is eliminated");
    }

    // Canonicity: distinct spellings of the same function share a root.
    checkCanonical("p -> q", "~p | q", {"p", "q"});
    checkCanonical("~(p & q)", "~p | ~q", {"p", "q"});
    checkCanonical("p <-> q", "(p & q) | (~p & ~q)", {"p", "q"});

    std::cout << g_checks << " checks, " << g_failures << " failure(s)\n";
    if (g_failures == 0) std::cout << "ALL TESTS PASSED\n";
    return g_failures == 0 ? 0 : 1;
}
