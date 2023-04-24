# wasmem

## Summary
Run two-dimensional finite-difference time-domain electromagnetic simulations directly in the browser. Interact with the simulation using keyboard and mouse actions. The simulation is written in `C++`, compiled to `WASM`, and the application is orchestrated in `JS`.

## Usage

### Web
Click this link: ...

### Locally
Clone repo. Run `./build.sh` and then `./run-html.sh` (or e.g. `./run-html.sh wsl-edge` to select another browser).

### User guide
Blah.

## TODO
- enable correct boundary conditions: periodic, reflective, and eventually PML
- evaluate/visualize total EM field energy density
- enable point and line sources with specified frequency and duration
- speed up main rasterizer (how?)
- have a little ruler in the image which shows the scale of the spatial domain
- keypress for simulation state reset (time=0, cleared fields; but no edits to domain spec)

## References
- https://en.wikipedia.org/wiki/Finite-difference_time-domain_method
- https://eecs.wsu.edu/~schneidj/ufdtd/
