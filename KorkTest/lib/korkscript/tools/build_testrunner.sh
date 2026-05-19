#!/usr/bin/env bash
set -euo pipefail

# Usage: ./tools/build_testrunner.sh [build_dir]
# Defaults to "test_build" if not provided.

BUILD_DIR="${1:-${CURRENT_BUILD_DIR:-test_build}}"
echo "BUILD DIR IS ${BUILD_DIR}"

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
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -- -j"$(cpu_count)"
