#!/bin/bash
rm -rf payload
rm -f wasmem.wasm;
emcc wasmem.cpp -s STANDALONE_WASM -fno-exceptions -DNDEBUG -std=c++14 -Wall -O3 -msimd128 --no-entry -o wasmem.wasm;

mkdir payload
mv wasmem.wasm payload/.
cp wasmem.js payload/.
cp index.html payload/.
