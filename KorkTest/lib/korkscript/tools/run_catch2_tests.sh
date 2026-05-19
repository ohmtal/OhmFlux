#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(git rev-parse --show-toplevel)"
cd "$ROOT_DIR"

CURRENT_BUILD_DIR="${CURRENT_BUILD_DIR:-test_build}"
TEST_RUNNER_BIN_NAME="${TEST_RUNNER_BIN_NAME:-testrunner}"
TEST_RUNNER_BIN="$ROOT_DIR/$CURRENT_BUILD_DIR/$TEST_RUNNER_BIN_NAME"

if [ ! -x "$TEST_RUNNER_BIN" ]; then
  echo "Missing test runner binary: $TEST_RUNNER_BIN" >&2
  exit 1
fi

exec "$TEST_RUNNER_BIN" catch2 "$@"
