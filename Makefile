CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Iinclude
SRC = src/lexer.cpp src/parser.cpp src/codegen.cpp src/compiler.cpp src/main.cpp
BIN = bin/veyra

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)
	@echo "✓ Built Veyra Compiler at $(BIN)"

install: $(BIN)
	@mkdir -p ~/.local/bin
	cp $(BIN) ~/.local/bin/veyra
	@echo "✓ Installed Veyra to ~/.local/bin/veyra"

clean:
	rm -rf bin build
