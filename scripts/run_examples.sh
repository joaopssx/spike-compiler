#!/usr/bin/env bash

set -euo pipefail

cmake -S . -B build
cmake --build build

./build/spike run examples/hello.por
./build/spike run examples/if.por
./build/spike run examples/loop.por
./build/spike run examples/fibonacci.por
