# Kaleidoscope Modern C++ / LLVM Compiler

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-orange.svg)](https://cmake.org/)
[![LLVM](https://img.shields.io/badge/LLVM-Supported-green.svg)](https://llvm.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A clean, production-grade C++ compiler front-end and optimization pipeline for the **Kaleidoscope** language, engineered with modern C++17 paradigms, modular compiler architecture, and direct LLVM Intermediate Representation (IR) generation.

Developed as a compiler engineering project demonstrating core compiler design principles suitable for high-performance compiler roles (such as CPU Compiler Engineering at NVIDIA).

---

## Architecture Overview

```
                      +-------------------+
                      |   Source Code     |  (.ks file or REPL)
                      +---------+---------+
                                |
                                v
                      +-------------------+
                      |      Lexer        |  Streaming tokenization, SourceLocation tracking
                      +---------+---------+
                                |  Tokens
                                v
                      +-------------------+
                      |      Parser       |  Pratt-style precedence climbing, syntax error sync
                      +---------+---------+
                                |  AST (Abstract Syntax Tree)
                                v
               +----------------+----------------+
               |                                 |
               v                                 v
     +-------------------+             +-------------------+
     |   AST Optimizer   |             |    AST Dumper     |  (--dump-ast tree printer)
     | (Constant Folding,|             +-------------------+
     | Algebraic Identity|
     | Dead Branch Elim) |
     +---------+---------+
               | Optimized AST
               v
     +-------------------+
     |   IR Generator    |  LLVM IR Code Generation (SSA construction, PHI nodes)
     +---------+---------+
               | LLVM IR
               v
     +-------------------+
     | LLVM Pass Pipeline|  Function passes (InstCombine, Reassociate, GVN, SimplifyCFG)
     +---------+---------+
               |
               v
     +-------------------+
     | Executable / JIT  |  (--emit-llvm or native JIT execution)
     +-------------------+
```

---

## Key Compiler Features

1. **Robust Streaming Lexer:**
   - Full `SourceLocation` (line, column) and `SourceRange` tracking.
   - Stream-agnostic design (`std::istream` abstraction) supporting interactive terminal inputs, file streams, and memory buffers without standard input side-effects.
   - Support for floating-point numeric literals, identifiers, language keywords, operators, and comment skipping (`# ...`).

2. **Recursive Descent & Precedence Climbing Parser:**
   - Operator precedence climbing algorithm for binary expressions (`+`, `-`, `*`, `/`, `<`, `>`).
   - Prefix unary operator support (`-x`, `+x`).
   - Rich control flow constructs: `if <cond> then <then> else <else>` and `for <var> = <start>, <end>, <step> in <body>`.
   - Comprehensive error recovery via `synchronize()` to continue parsing subsequent top-level definitions upon syntax errors.

3. **Production Diagnostic Engine:**
   - Clang/GCC-style diagnostic reporting (`filename:line:col: error: message`).
   - Source snippet extraction with column-accurate caret (`^`) pointing.
   - Tracks error and warning counts to reliably halt compilation before invalid code emission.

4. **Object-Oriented AST & Visitor Pattern:**
   - Polymorphic expression hierarchy (`ExprAST`) with clear `std::unique_ptr` ownership semantics.
   - Decoupled traversals using the Visitor design pattern (`ASTVisitor`).
   - Tree dumper (`ASTDumper`) emitting hierarchical S-expression visualizations.

5. **AST-Level Optimization Passes (`ASTConstantFolder`):**
   - **Constant Folding:** Compile-time evaluation of constant expressions (e.g. `2.0 + 3.0 * 4.0` -> `14.0`).
   - **Algebraic Identity Simplification:**
     - Addition identity: `x + 0` -> `x`, `0 + x` -> `x`
     - Subtraction identity: `x - 0` -> `x`
     - Multiplication identity: `x * 1` -> `x`, `1 * x` -> `x`
     - Annihilator: `x * 0` -> `0`, `0 * x` -> `0`
     - Division identity: `x / 1` -> `x`
   - **Dead Branch Elimination:** Statically evaluates if-condition constants to eliminate unreachable branches prior to IR emission.

6. **LLVM Intermediate Representation (IR) Generation:**
   - Emits valid SSA (Static Single Assignment) form.
   - Constructs basic blocks with `llvm::BasicBlock` and `llvm::IRBuilder<>`.
   - Implements conditional branching and loop constructs using explicit `llvm::PHINode` instances for phi-placement.
   - Dual-backend architecture: Native LLVM C++ API code generation when LLVM is installed, and a portable textual IR emitter for standalone builds.

7. **LLVM Middle-End Optimization Pipeline:**
   - Configurable optimization levels (`-O0`, `-O1`, `-O2`, `-O3`).
   - Optimization passes:
     - **Instruction Combining (`InstCombine`):** Peephole transformations.
     - **Expression Reassociation (`Reassociate`):** Commutative and associative expression rearrangement.
     - **Global Value Numbering (`GVN`):** Common Subexpression Elimination (CSE).
     - **CFG Simplification (`SimplifyCFG`):** Pruning unreachable basic blocks and merging contiguous blocks.

---

## Project Structure

```
kaleidoscope/
├── CMakeLists.txt                 # Modern target-based CMake build configuration
├── LICENSE                        # MIT License (Astha Jha 2026)
├── README.md                      # Architecture documentation & interview guide
├── .gitignore                     # Git ignore rules for CMake, build files, and IDEs
│
├── include/kaleidoscope/          # Public compiler headers
│   ├── Common.hpp                 # SourceLocation, SourceRange, and utility types
│   ├── Diagnostic.hpp             # Rich error diagnostics and caret rendering
│   ├── Token.hpp                  # Token kinds, literals, and precedence mapping
│   ├── Lexer.hpp                  # Streaming Lexer with line/col tracking
│   ├── AST.hpp                    # Object-oriented AST node hierarchy
│   ├── ASTVisitor.hpp             # Visitor interface and AST pretty printer
│   ├── ASTOptimizer.hpp           # AST-level Constant Folding & Algebraic passes
│   ├── Parser.hpp                 # Recursive descent parser with precedence climbing
│   ├── CodeGen.hpp                # LLVM IR code generator with type checking
│   ├── Optimizer.hpp              # Pass pipeline management
│   └── Driver.hpp                 # Driver coordinating stages and CLI options
│
├── src/                           # Compiler implementation files
│   ├── Diagnostic.cpp
│   ├── Token.cpp
│   ├── Lexer.cpp
│   ├── AST.cpp
│   ├── ASTVisitor.cpp
│   ├── ASTOptimizer.cpp
│   ├── Parser.cpp
│   ├── CodeGen.cpp
│   ├── Optimizer.cpp
│   ├── Driver.cpp
│   └── main.cpp                   # Compiler CLI entry point
│
├── tests/                         # Automated unit & regression test suite
│   ├── CMakeLists.txt
│   ├── test_main.cpp              # Test harness runner
│   ├── test_lexer.cpp             # Lexer token and location tests
│   ├── test_parser.cpp            # Parser precedence, control flow, and syntax error tests
│   ├── test_ast_optimizer.cpp     # Constant folding and algebraic reduction tests
│   └── test_codegen.cpp           # LLVM IR generation and block verification tests
│
└── examples/                      # Canonical Kaleidoscope sample programs
    ├── fibonacci.ks               # Recursive Fibonacci algorithm
    ├── factorial.ks               # Recursive factorial
    ├── mandelbrot.ks              # ASCII Mandelbrot set visualizer using putchard()
    └── control_flow.ks            # Conditionals, for-loops, and unary operations
```

---

## Building and Installation

### Prerequisites
* **CMake:** Version 3.20 or newer
* **C++ Compiler:** Supporting C++17 (e.g. GCC 9+, Clang 10+, or MSVC 2019+)
* **LLVM (Optional):** LLVM development libraries (if building with native LLVM JIT/Code generation)

### Build Commands

```bash
# 1. Configure the project
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build the compiler library, CLI executable, and tests
cmake --build build --config Release

# 3. Run the automated test suite
ctest --test-dir build --output-on-failure
```

---

## Compiler Usage & CLI Options

```bash
# Launch interactive REPL mode
./build/kaleidoscope

# Compile a Kaleidoscope source file and emit LLVM IR
./build/kaleidoscope --emit-llvm examples/fibonacci.ks

# Dump the Abstract Syntax Tree representation
./build/kaleidoscope --dump-ast examples/factorial.ks

# Compile with optimization level 2 and emit IR
./build/kaleidoscope -O2 --emit-llvm examples/control_flow.ks

# Compile without AST-level constant folding
./build/kaleidoscope --no-ast-opt --emit-llvm examples/control_flow.ks
```

### CLI Command Reference
| Option | Description |
| :--- | :--- |
| `-h`, `--help` | Display command-line help message |
| `-v`, `--version` | Display compiler version and backend configuration |
| `--dump-ast` | Print formatted Abstract Syntax Tree representation |
| `--emit-llvm` | Emit LLVM Intermediate Representation (IR) |
| `--no-ast-opt` | Disable front-end AST constant folding passes |
| `-O0` | Disable optimization passes |
| `-O1` | Basic peephole and CFG simplifications |
| `-O2` | Standard optimization pipeline (InstCombine, Reassociate, GVN, SimplifyCFG) |
| `-O3` | Aggressive optimization passes |

---

## Language Grammar (EBNF)

```ebnf
program        ::= ( definition | external | expression | ';' )*

definition     ::= 'def' prototype expression
external       ::= 'extern' prototype
prototype      ::= identifier '(' identifier* ')'

expression     ::= unary ( binop unary )*
unary          ::= ( '+' | '-' ) unary | primary

primary        ::= identifier
                 | identifier '(' ( expression ( ',' expression )* )? ')'
                 | number
                 | '(' expression ')'
                 | if_expr
                 | for_expr

if_expr        ::= 'if' expression 'then' expression 'else' expression
for_expr       ::= 'for' identifier '=' expression ',' expression ( ',' expression )? 'in' expression

binop          ::= '+' | '-' | '*' | '/' | '<' | '>'
```

---

## Technical Interview Guide: Compiler Engineering (NVIDIA Focus)

When discussing this repository in a compiler engineering interview, key architectural decisions and trade-offs to highlight include:

### 1. Why use Operator Precedence Climbing over full LR/LALR parsers?
* **Interview Insight:** Operator Precedence Climbing is a variant of Pratt parsing. It parses binary expressions in $O(N)$ time with minimal stack depth, avoids shifting/reducing state table overhead, and makes adding new operators with specific precedence and associativity trivial without grammar conflicts.

### 2. Why perform Constant Folding at the AST level rather than deferring entirely to LLVM?
* **Interview Insight:** Performing front-end constant folding shrinks the size of the generated Abstract Syntax Tree, eliminates dead code branches (such as unreachable `else` blocks in `if 1 then ... else ...`), reduces memory usage, and minimizes the volume of LLVM IR instructions that the LLVM pass pipeline must ingest, accelerating compile times.

### 3. How does the compiler handle SSA Form and PHI nodes for control flow?
* **Interview Insight:** Kaleidoscope uses a pure expression-oriented model. For `if-then-else` expressions:
  1. The condition basic block evaluates the condition and conditionally branches to `then` and `else` blocks.
  2. The `then` and `else` blocks compute their respective values and unconditionally branch to a single `merge` block.
  3. The `merge` basic block inserts a `llvm::PHINode` combining the incoming values from the two predecessor blocks.
  4. For `for` loops, a PHI node is placed at the loop header for the induction variable, taking the start value from the preheader and the updated step value from the loop backedge.

### 4. Memory Management: Unique Pointers vs. Arena Allocators
* **Interview Insight:** In this implementation, `std::unique_ptr` enforces strict ownership semantics and prevents resource leaks. In a production industrial compiler (such as Clang or NVIDIA's NVVM/PTX compiler), an **Arena / Bump Allocator** (`llvm::BumpPtrAllocator`) is commonly favored for AST nodes because individual nodes do not need to be freed independently—freeing the entire arena at the end of translation unit compilation eliminates allocation fragmentation and destructor call overhead.

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

* **Author & Maintainer:** Astha Jha ([@astha-2004-code](https://github.com/astha-2004-code)) - Kaleidoscope Modern C++ Compiler, AST optimizer, and LLVM optimization pipeline.

