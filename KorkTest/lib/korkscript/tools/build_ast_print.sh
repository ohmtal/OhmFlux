#!/usr/bin/env bash
set -euo pipefail

# Usage: ./tools/build_csprintast.sh [build_dir]
# Defaults to "build" if not provided.

BUILD_DIR="${1:-build}"
echo "BUILD DIR IS ${BUILD_DIR}"
CS_BIN_NAME="${CS_BIN_NAME:-csprintast}"

cpu_count() {
  if command -v getconf >/dev/null 2>&1; then
    getconf _NPROCESSORS_ONLN 2>/dev/null && return 0
  fi
  if command -v sysctl >/dev/null 2>&1; then
    sysctl -n hw.ncpu 2>/dev/null && return 0
  fi
  echo 4
}

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j"$(cpu_count)"
