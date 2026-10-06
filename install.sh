#!/usr/bin/env bash
set -e

echo -e "\033[1;36m==> Installing Veyra Language Compiler...\033[0m"

INSTALL_DIR="${HOME}/.local/bin"
mkdir -p "${INSTALL_DIR}"

make -j$(nproc)
cp bin/veyra "${INSTALL_DIR}/veyra"

echo -e "\033[1;32m✓ Veyra installed successfully to ${INSTALL_DIR}/veyra\033[0m"
echo "Make sure ${INSTALL_DIR} is in your PATH."
echo "Usage: veyra main.vey"
