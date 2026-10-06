# 🌌 Veyra (`.vey`)

> **Simple as Python. Powerful as C++.**

Veyra is a modern systems and game programming language that delivers the raw power, zero garbage collection pauses, and memory model of C++20, but with the concise, noise-free ergonomics of Python.

---

## ⚡ Highlights

- **1 Single Command to Run**: Just type `veyra app.vey` — it transpiles, caches, compiles, and runs instantly in one step (just like `python app.py`).
- **Zero `std::` or `#include` Ceremony**: A rich, high-performance prelude is embedded directly into the compiler binary.
- **50% Less Code**: No headers, no manual memory boilerplate, intuitive list literals `[1, 2, 3]`, ranges `0..10`, and string interpolation `"{var}"`.
- **Game Dev Ready**: Zero GC stutter, value semantics, and seamless interop with Raylib, SDL, OpenGL, Box2D, and C/C++ libraries.
- **Single-Binary Shipping**: The `veyra` compiler is a standalone binary with embedded prelude — anyone can download and run it immediately.

---

## 📦 Quick Installation

```bash
# Build and install to ~/.local/bin/veyra
git clone https://github.com/IIXII-L192/veyra.git
cd veyra
./install.sh
```

---

## 🚀 Usage

### 1. Run a script directly (1 command):
```bash
veyra app.vey
# or
veyra run app.vey
```

### 2. Compile to standalone optimized native binary:
```bash
veyra build app.vey -o myapp -O3
```

### 3. Inspect generated C++20 code:
```bash
veyra emit app.vey
```

### 4. Create a new project:
```bash
veyra new my_project
```

---

## 📝 Syntax Quickstart

### Hello World & String Interpolation
```veyra
fn main() {
    let name = "Developer"
    let year = 2026
    println("Hello {name}, welcome to {year}!")
}
```

### Loops, Lists & Functions
```veyra
fn square(x: int) => x * x

fn main() {
    let scores = [95, 88, 72, 100, 64]
    
    for i, score in enumerate(scores) {
        println("Player {i + 1}: {score} (squared: {square(score)})")
    }

    for i in 1..6 {
        println("Step: {i}")
    }
}
```

### Structs & Game Simulation
```veyra
struct Vector2D {
    x: float = 0.0
    y: float = 0.0

    fn length() => sqrt(x * x + y * y)
}

struct Player {
    name: string
    pos: Vector2D
    health: int = 100
}

fn main() {
    mut hero = Player("Shadow Knight", Vector2D(100.0, 50.0), 100)
    hero.pos.x += 15.0
    println("Hero {hero.name} is at distance: {hero.pos.length()}")
}
```

---

## 📂 Project Structure

- `include/veyra/`: AST, Lexer, Parser, Code Generator, Prelude, Compiler Driver
- `src/`: Core implementation files
- `examples/`: Ready-to-run `.vey` programs
- `install.sh`: 1-command installer
