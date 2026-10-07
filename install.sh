#!/usr/bin/env bash
# Veyra Universal Automated Installer for Linux & macOS
# Compiles directly from GitHub source archive without requiring releases.
set -e

COLOR_PURPLE="\033[1;35m"
COLOR_CYAN="\033[1;36m"
COLOR_GREEN="\033[1;32m"
COLOR_YELLOW="\033[1;33m"
COLOR_RED="\033[1;31m"
COLOR_RESET="\033[0m"

echo -e "${COLOR_PURPLE}"
echo " __      __                          "
echo " \ \    / /__ _   _ _ __ __ _       "
echo "  \ \  / / _ \ | | | '__/ _\` |      "
echo "   \ \/ /  __/ |_| | | | (_| |      "
echo "    \__/ \___|\__, |_|  \__,_|      "
echo "               |___/  INSTALLER     "
echo -e "${COLOR_RESET}"

INSTALL_BIN="${HOME}/.local/bin"
INSTALL_INC="${HOME}/.local/include/veyra"
mkdir -p "${INSTALL_BIN}" "${INSTALL_INC}"

OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"

echo -e "${COLOR_CYAN}==> Detected Environment:${COLOR_RESET} ${OS} (${ARCH})"

# Determine C++ Compiler
CXX_COMPILER=""
if [ -n "${CXX}" ] && command -v "${CXX}" >/dev/null 2>&1; then
    CXX_COMPILER="${CXX}"
elif command -v g++ >/dev/null 2>&1; then
    CXX_COMPILER="g++"
elif command -v clang++ >/dev/null 2>&1; then
    CXX_COMPILER="clang++"
elif command -v c++ >/dev/null 2>&1; then
    CXX_COMPILER="c++"
fi

if [ -z "${CXX_COMPILER}" ]; then
    echo -e "${COLOR_RED}Error: A C++20 capable compiler (g++ >= 11 or clang++ >= 13) is required to build Veyra.${COLOR_RESET}"
    echo "Please install a C++ compiler:"
    echo "  Ubuntu/Debian: sudo apt update && sudo apt install -y build-essential g++"
    echo "  Arch Linux:    sudo pacman -S base-devel"
    echo "  Fedora:        sudo dnf groupinstall 'Development Tools'"
    echo "  macOS:         xcode-select --install"
    exit 1
fi

echo -e "${COLOR_CYAN}==> Using C++ Compiler:${COLOR_RESET} ${CXX_COMPILER}"

# Build temporary directory
TMP_DIR="$(mktemp -d -t veyra-install-XXXXXX)"
trap 'rm -rf "${TMP_DIR}"' EXIT

echo -e "${COLOR_CYAN}==> Fetching latest Veyra source repository...${COLOR_RESET}"
SOURCE_TAR_URL="https://github.com/IIXII-L192/veyra/archive/refs/heads/main.tar.gz"

if command -v curl >/dev/null 2>&1; then
    curl -fsSL "${SOURCE_TAR_URL}" | tar -xz -C "${TMP_DIR}"
elif command -v wget >/dev/null 2>&1; then
    wget -qO- "${SOURCE_TAR_URL}" | tar -xz -C "${TMP_DIR}"
else
    echo -e "${COLOR_RED}Error: curl or wget is required to download Veyra.${COLOR_RESET}"
    exit 1
fi

SRC_DIR="${TMP_DIR}/veyra-main"

echo -e "${COLOR_CYAN}==> Compiling native Veyra binary with -O3 optimization...${COLOR_RESET}"
cd "${SRC_DIR}"

${CXX_COMPILER} -std=c++20 -O3 -Iinclude src/lexer.cpp src/parser.cpp src/codegen.cpp src/main.cpp -o "${INSTALL_BIN}/veyra"

# Copy headers
if [ -d "include/veyra" ]; then
    cp -r include/veyra/* "${INSTALL_INC}/" 2>/dev/null || true
elif [ -d "include" ]; then
    cp -r include/* "${INSTALL_INC}/" 2>/dev/null || true
fi

chmod +x "${INSTALL_BIN}/veyra"

# Configure PATH in shell config if needed
SHELL_CONFIG=""
if [ -n "${ZSH_VERSION}" ] || [ -f "${HOME}/.zshrc" ]; then
    SHELL_CONFIG="${HOME}/.zshrc"
elif [ -f "${HOME}/.bashrc" ]; then
    SHELL_CONFIG="${HOME}/.bashrc"
elif [ -f "${HOME}/.profile" ]; then
    SHELL_CONFIG="${HOME}/.profile"
fi

if [ -n "${SHELL_CONFIG}" ]; then
    if ! grep -q '\.local/bin' "${SHELL_CONFIG}"; then
        echo 'export PATH="${HOME}/.local/bin:${PATH}"' >> "${SHELL_CONFIG}"
        echo -e "${COLOR_CYAN}==> Added ~/.local/bin to ${SHELL_CONFIG}${COLOR_RESET}"
    fi
fi

echo ""
echo -e "${COLOR_GREEN}✓ Veyra successfully compiled and installed!${COLOR_RESET}"
echo -e "  Binary:  ${INSTALL_BIN}/veyra"
echo -e "  Prelude: ${INSTALL_INC}/"
echo ""
echo "Verify installation:"
echo "  export PATH=\"\${HOME}/.local/bin:\${PATH}\""
echo "  veyra --version"
echo ""
echo "Run your first script:"
echo "  echo 'println(\"Hello, Veyra!\")' > hello.vey"
echo "  veyra run hello.vey"
