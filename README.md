# Formula2BDD: Visual Boolean Formula Compiler
Formula2BDD takes a propositional logic formula like
```
(p ∨ (q ∧ r)) → ¬s
```
and compiles it into a Reduced Ordered Binary Decision Diagram (ROBDD). An ROBDD is a canonical, compressed DAG that behaves exactly like the original formula, just in a much more compact form. Once it's built you can export it as Graphviz DOT or JSON.
## Pipeline
```
formula string
   └─ Lexer      → tokens          (Unicode ¬∧∨⊕→↔  or ASCII ~ ! & | ^ -> <->)
       └─ Parser → AST             (recursive descent, operator precedence)
           └─ BddManager → ROBDD   (Shannon expansion + hash-consed unique table)
               └─ Exporters → .dot / .json
```
The ROBDD is built by Shannon expansion (`f = ¬x·f|ₓ₌₀ + x·f|ₓ₌₁`) down a fixed variable order. As nodes are created, a unique table applies the two reduction rules that keep the result canonical:
1. **Redundant-node elimination.** If a node's `low == high`, drop it.
2. **Isomorphic-subgraph merging.** Identical `(var, low, high)` triples are shared instead of duplicated.
Because of these rules, two formulas are logically equivalent if and only if they build the identical ROBDD. The tests rely on exactly this property.
## Build
You'll need a C++17 compiler. There are two ways to build:
```bash
make            # builds ./build/formula2bdd
make test       # builds and runs the test suite
# or with CMake:
cmake -B build && cmake --build build
ctest --test-dir build
```
## Usage
```bash
# DOT to stdout, with node-count stats on stderr
./build/formula2bdd "(p | (q & r)) -> ~s" --format dot --stats
# JSON to a file
./build/formula2bdd "p <-> q" --format json --out bdd.json
# Custom variable ordering (this can change the size a lot)
./build/formula2bdd "(a & b) | (c & d)" --order a,c,b,d --stats
# Evaluate one assignment and trace the path to a terminal
./build/formula2bdd "(p | (q & r)) -> ~s" --eval "p=1,q=0,r=0,s=1"
```
### Rendering a diagram
The DOT output renders with Graphviz (`brew install graphviz`):
```bash
./build/formula2bdd "(p | (q & r)) -> ~s" --out out.dot
dot -Tpng out.dot -o out.png    # or -Tsvg
```
Solid edges are the `1`/high branch and dashed edges are the `0`/low branch. The boxes are the `True`/`False` terminals.
## Options
| Option | Description |
|---|---|
| `--order a,b,c` | Fix the variable ordering (defaults to order of first appearance) |
| `--format dot\|json` | Output format (default `dot`) |
| `--out FILE` | Write to a file instead of stdout |
| `--eval a=1,b=0` | Evaluate a single assignment and print the traced path |
| `--stats` | Print live/unique node counts to stderr |
| `--help` | Show usage |
## Layout
```
include/formula2bdd/   public headers (Token, Lexer, Ast, Parser, Evaluator,
                       BddManager, Exporters)
src/                   implementations
app/main.cpp           CLI
tests/                 brute-force oracle + canonicity tests
```

