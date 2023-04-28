# wasmem

## Summary
Run two-dimensional finite-difference time-domain electromagnetic simulations directly in the browser. Interact with the simulation using keyboard and mouse actions. At this time, the simulator is limited to $\mathrm{TM}^z$ polarization and visualizes the $z$-component of the electric field: $E_z(x,y)$. The $x$ and $y$ components of the $B$ field are also part of the simulation state but not (yet) rasterized. The simulation is written in `C++`, compiled to `WASM`, and the application is orchestrated in `JS`.

Run directly in browser: https://raw.githack.com/olofer/wasmem/main/payload/index.html

## Usage
- `R` reset simulation state (zero fields, reset source)
- `P` pause/unpause field updater (but not rendering)
- `X` toggle periodic boundary in x (horizontal) direction
- `Y` toggle periodic boundary in y (vertical) direction
- `C` set color range to current field range, or go back to source range
- `D` toggle medium conductivity (damping effect) 
- `+/-` change source frequency (i.e. points per wavelength)
- `up/down` and `left/right` move source location
- `A` toggle additive/absolute source injection
- `pgup/pgdown` increase or decrease the medium skin length (damping)
- `0` turn off source (no source)
- `1` sinusoidal continuous source
- `2` pulsed Ricker wavelet source
- `3` square wave source (unresolved)
- `4` sawtooth wave source (unresolved)
- `G` superimpose a Gaussian into the domain center
- `Z` toggle test rasterizer screen (see full colormap)
- `S` toggle display of simulation information text

The source location can be placed directly at the cursor with a mouse click.

### Local build & run
Clone repo. Run `./build.sh` and then `./run-html.sh` (or e.g. `./run-html.sh wsl-edge` to select another browser, assuming WSL2 environment). For the build to succeed, the `emscripten` is requried (see link below).

## References
- https://en.wikipedia.org/wiki/Finite-difference_time-domain_method
- https://eecs.wsu.edu/~schneidj/ufdtd/
- `emscripten`: https://github.com/emscripten-core/emsdk

## TODO
- [ ] absorbing boundaries
- [ ] medium editor
- [ ] rasterizer speed up
- [ ] more source options & bandlimit 
- [ ] visualization of $H$, and $u_E,u_B$
