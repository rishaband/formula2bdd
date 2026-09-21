#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "formula2bdd/BddManager.hpp"
#include "formula2bdd/Evaluator.hpp"
#include "formula2bdd/Exporters.hpp"
#include "formula2bdd/Parser.hpp"

using namespace f2b;

namespace {

void printUsage(const char* prog) {
    std::cout <<
        "Formula2BDD - compile a propositional formula into an ROBDD\n\n"
        "Usage:\n"
        "  " << prog << " \"<formula>\" [options]\n\n"
        "Operators (Unicode or ASCII):\n"
        "  NOT ¬ ~ !   AND ∧ &   OR ∨ |   XOR ⊕ ^   IMP → ->   IFF ↔ <->\n"
        "  Constants: 0 (false), 1 (true).  Grouping: ( )\n\n"
        "Options:\n"
        "  --order a,b,c     Fix the variable ordering (default: first appearance)\n"
        "  --format FMT      Output format: dot (default) or json\n"
        "  --out FILE        Write output to FILE instead of stdout\n"
        "  --eval a=1,b=0    Evaluate one assignment and trace the path\n"
        "  --stats           Print node-count statistics to stderr\n"
        "  --help            Show this help\n\n"
        "Examples:\n"
        "  " << prog << " \"(p | (q & r)) -> ~s\" --format dot --out out.dot\n"
        "  " << prog << " \"p <-> q\" --eval p=1,q=0\n";
}

std::vector<std::string> splitCsv(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream is(s);
    while (std::getline(is, cur, ',')) {
        // trim spaces
        size_t a = cur.find_first_not_of(" \t");
        size_t b = cur.find_last_not_of(" \t");
        if (a != std::string::npos) out.push_back(cur.substr(a, b - a + 1));
    }
    return out;
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) { printUsage(argv[0]); return 0; }

    std::string formula;
    std::string orderArg;
    std::string format = "dot";
    std::string outFile;
    std::string evalArg;
    bool stats = false;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        auto need = [&](const char* name) -> std::string {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: " << name << " requires an argument\n";
                std::exit(2);
            }
            return args[++i];
        };
        if (a == "--help" || a == "-h") { printUsage(argv[0]); return 0; }
        else if (a == "--order")  orderArg = need("--order");
        else if (a == "--format") format = need("--format");
        else if (a == "--out")    outFile = need("--out");
        else if (a == "--eval")   evalArg = need("--eval");
        else if (a == "--stats")  stats = true;
        else if (!a.empty() && a[0] == '-') {
            std::cerr << "Error: unknown option '" << a << "'\n";
            return 2;
        } else {
            formula = a; // positional formula
        }
    }

    if (formula.empty()) {
        std::cerr << "Error: no formula given.\n";
        return 2;
    }

    AstPtr ast;
    try {
        ast = parseFormula(formula);
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return 1;
    }

    // Determine variable order.
    std::vector<std::string> appearing = variablesOf(ast.get());
    std::vector<std::string> order;
    if (!orderArg.empty()) {
        order = splitCsv(orderArg);
        // Every appearing variable must be covered by the order.
        for (const auto& v : appearing) {
            if (std::find(order.begin(), order.end(), v) == order.end()) {
                std::cerr << "Error: variable '" << v
                          << "' is used in the formula but missing from --order\n";
                return 1;
            }
        }
    } else {
        order = appearing;
    }

    BddManager mgr(order);
    NodeId root = mgr.build(ast.get());

    // --eval: evaluate a single assignment and trace the path.
    if (!evalArg.empty()) {
        Assignment a;
        for (const auto& kv : splitCsv(evalArg)) {
            auto eq = kv.find('=');
            if (eq == std::string::npos) {
                std::cerr << "Error: bad --eval entry '" << kv << "' (want var=0/1)\n";
                return 1;
            }
            std::string name = kv.substr(0, eq);
            std::string val = kv.substr(eq + 1);
            a[name] = (val == "1" || val == "true" || val == "T");
        }
        for (const auto& v : order) if (!a.count(v)) a[v] = false;

        std::vector<NodeId> trace;
        bool result = mgr.evalPath(root, a, &trace);

        std::cout << "Result: " << (result ? "True" : "False") << "\n";
        std::cout << "Path:   ";
        for (size_t i = 0; i < trace.size(); ++i) {
            NodeId id = trace[i];
            if (mgr.isTerminal(id)) std::cout << (id == kTrue ? "True" : "False");
            else std::cout << mgr.varName(mgr.node(id).varIndex);
            if (i + 1 < trace.size()) std::cout << " -> ";
        }
        std::cout << "\n";

        // Sanity check against the direct evaluator.
        bool oracle = evaluate(ast.get(), a);
        if (oracle != result) {
            std::cerr << "INTERNAL ERROR: BDD disagrees with evaluator!\n";
            return 3;
        }
        return 0;
    }

    // Otherwise: emit the graph.
    std::string out;
    if (format == "dot") out = toDot(mgr, root, formula);
    else if (format == "json") out = toJson(mgr, root);
    else { std::cerr << "Error: unknown --format '" << format << "'\n"; return 2; }

    if (outFile.empty()) {
        std::cout << out;
    } else {
        std::ofstream f(outFile);
        if (!f) { std::cerr << "Error: cannot write to '" << outFile << "'\n"; return 1; }
        f << out;
        std::cerr << "Wrote " << format << " to " << outFile << "\n";
    }

    if (stats) {
        std::cerr << "Variables:   " << order.size() << "\n";
        std::cerr << "Live nodes:  " << mgr.liveNodeCount(root) << " decision node(s)\n";
        std::cerr << "Table nodes: " << mgr.tableSize() << " unique node(s) created\n";
    }
    return 0;
}
