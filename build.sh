#!/bin/bash
rm -rf payload
rm -f wasmem.wasm;
emcc wasmem.cpp -s STANDALONE_WASM -fno-exceptions -DNDEBUG -std=c++17 -Wall -O3 --no-entry -o wasmem.wasm;

mkdir payload
mv wasmem.wasm payload/.
cp wasmem.js payload/.
cp index.html payload/.
