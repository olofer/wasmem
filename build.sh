#!/bin/bash
rm -rf payload
rm -f wasmem.wasm;
emcc -std=c++17 -Wall -O2 --no-entry -s STANDALONE_WASM wasmem.cpp -o wasmem.wasm;

mkdir payload
mv wasmem.wasm payload/.
cp wasmem.js payload/.
cp index.html payload/.
