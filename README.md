# wasmem

## Summary
Run two-dimensional finite-difference time-domain electromagnetic simulations directly in the browser. Interact with the simulation using keyboard and mouse actions. The simulation is written in `C++`, compiled to `WASM`, and the application is orchestrated in `JS`.

Run directly in browser: https://raw.githack.com/olofer/wasmem/main/payload/index.html

## Usage
- `R` reset simulation state (zero fields, reset source)
- `P` pause/unpause field updater (but not rendering)
- `X` toggle periodic boundary in x (horizontal) direction
- `Y` toggle periodic boundary in y (vertical) direction
- `Z` toggle test rasterizer screen (see full colormap)
- `C` set color range to current field range, or go back to source range
- `D` toggle medium conductivity (damping effect) 
- `+/-` change source frequency (i.e. points per wavelength)
- `up/down` and `left/right` move source location
- `0` turn off source (no source)
- `1` sinusoidal continuous source
- `2` pulsed Ricker wavelet source
- `3` square wave source

The source location can be placed directly at the cursor with a mouse click.

### Local build & run
Clone repo. Run `./build.sh` and then `./run-html.sh` (or e.g. `./run-html.sh wsl-edge` to select another browser, assuming WSL2 environment).

## TODO
- [ ] support for absorbing boundaries
- [ ] evaluate/visualize total EM field energy density; or at least Hx, Hy
- [ ] speed up main rasterizer (how? floats?!)
- [ ] more sources, axis aligned lines? (and option to add or set source)

## References
- https://en.wikipedia.org/wiki/Finite-difference_time-domain_method
- https://eecs.wsu.edu/~schneidj/ufdtd/
