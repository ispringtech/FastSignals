#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

cd "${SCRIPT_DIR}"
git submodule update --init --recursive

mkdir -p "${SCRIPT_DIR}/tests/benchmark-bin" && cd "${SCRIPT_DIR}/tests/benchmark-bin"
cmake ../benchmark -DBENCHMARK_ENABLE_TESTING=OFF

