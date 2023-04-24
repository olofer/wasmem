# wasmem

## Summary
Run two-dimensional finite-difference time-domain electromagnetic simulations directly in the browser. Interact with the simulation using keyboard and mouse actions. The simulation is written in `C++`, compiled to `WASM`, and the application is orchestrated in `JS`.

## Usage
- `R` reset simulation state (zero fields, reset source)
- `P` pause/unpause field updater (but not rendering)
- `Z` toggle test rasterizer screen (see full colormap)

### Web
Click this link: (TBA)

### Locally
Clone repo. Run `./build.sh` and then `./run-html.sh` (or e.g. `./run-html.sh wsl-edge` to select another browser, assuming WSL2 environment).

### User guide
Blah.

## TODO
- keypress: 0,1,2,3.. for source type; arrows for moving source, +/- for change of PPW
- Ricker wavelet pulse: repetition rate; line source
- periodic, PEC, PMC, and ABC boundaries (dynamically selectable)
- evaluate/visualize total EM field energy density; or at least Hx, Hy
- speed up main rasterizer (how? floats?!)
- have a little ruler in the image which shows the scale of the spatial domain

## References
- https://en.wikipedia.org/wiki/Finite-difference_time-domain_method
- https://eecs.wsu.edu/~schneidj/ufdtd/
