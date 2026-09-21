# Formula2BDD — Visual Boolean Formula Compiler

Compiles a propositional logic formula such as

```
(p ∨ (q ∧ r)) → ¬s
```

into a **Reduced Ordered Binary Decision Diagram (ROBDD)** — a canonical, compressed DAG
with the same logical behavior — and exports it as Graphviz DOT or JSON.

## Pipeline

```
formula string
   └─ Lexer      → tokens          (Unicode ¬∧∨⊕→↔  or ASCII ~ ! & | ^ -> <->)
       └─ Parser → AST             (recursive descent, operator precedence)
           └─ BddManager → ROBDD   (Shannon expansion + hash-consed unique table)
               └─ Exporters → .dot / .json
```

The ROBDD is built by Shannon expansion (`f = ¬x·f|ₓ₌₀ + x·f|ₓ₌₁`) down a fixed
variable order. A unique table enforces the two reduction rules that make the
result canonical:

1. **Redundant-node elimination** — if a node's `low == high`, drop it.
2. **Isomorphic-subgraph merging** — identical `(var, low, high)` triples are shared.

Canonicity means two formulas are logically equivalent **iff** they build the
identical ROBDD (verified in the tests).

## Build

Requires a C++17 compiler. Two options:

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

# Custom variable ordering (dramatically affects size)
./build/formula2bdd "(a & b) | (c & d)" --order a,c,b,d --stats

# Evaluate one assignment and trace the path to a terminal
./build/formula2bdd "(p | (q & r)) -> ~s" --eval "p=1,q=0,r=0,s=1"
```

### Rendering a diagram

DOT output renders with Graphviz (`brew install graphviz`):

```bash
./build/formula2bdd "(p | (q & r)) -> ~s" --out out.dot
dot -Tpng out.dot -o out.png    # or -Tsvg
```

Solid edges are the `1`/high branch, dashed edges the `0`/low branch; boxes are
the `True`/`False` terminals.

## Options

| Option | Description |
|---|---|
| `--order a,b,c` | Fix the variable ordering (default: order of first appearance) |
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

## Notes & next steps

- Construction currently uses direct Shannon recursion, which enumerates 2ⁿ leaf
  assignments — fine for teaching-scale formulas. The classic memoized `apply`
  algorithm is the natural next step for scaling.
- Ideas: PNG/SVG shell-out built into the CLI, a raw (unreduced) decision-tree
  mode, dynamic variable reordering, and a D3.js interactive web view.
```
