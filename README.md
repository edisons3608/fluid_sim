# Fluid Simulation

A 2D fluid simulation built with C++ and SFML, showing  smoke visualization ontop pressure field rendering.

## Features

- **Real-time fluid dynamics simulation** using a grid-based approach
- **Interactive smoke visualization** with inflow/outflow conditions
- **Pressure field rendering** using scientific color mapping (blue → cyan → green → yellow → red)
- **Draggable boundary objects** that affect fluid flow
- **Configurable simulation parameters** (gravity, density, overrelaxation)
- **Efficient grid-based algorithms** for fluid propagation and incompressibility

## Screenshots

The simulation displays:
- **Smoke field**: White smoke particles flowing from left to right
- **Pressure field**: Color-coded pressure visualization using scientific color mapping
- **Interactive boundaries**: Draggable circular objects that create solid boundaries
- **Real-time updates**: 30 FPS simulation with continuous fluid flow

## Reqs

- **C++17** compatible compiler (GCC or Clang)
- **SFML 3.x** (graphics, window, and system components)
- **Make** or **CMake** for building

## Usage

### Running the Simulation
```bash
# build separately and run
make
./FluidSim
```


### Simulation Parameters

The simulation uses these default parameters:
- **Grid size**: 100x100 cells
- **Gravity**: 9.81 m/s²
- **Density**: 1.0 kg/m³
- **Overrelaxation**: 1.9 (for faster convergence)
- **Time step**: 1/60 second
- **Iterations**: 20 pressure solver iterations per frame


## Technical Details

### Fluid Simulation Algorithm
The simulation implements:
- **Grid-based fluid dynamics** using staggered grid layout
- **Pressure projection** for incompressibility
- **Advection** for fluid and smoke transport
- **Boundary handling** for solid obstacles
- **Gravity effects** on fluid motion

### Rendering
- **SFML graphics** for real-time visualization
- **Scientific color mapping** for pressure field display
- **Smoke blending** with pressure colors for efficient rendering
- **Grid-based cell rendering** for performance

### Performance
- **Optimized grid operations** with minimal memory allocation
- **Configurable iteration counts** for pressure solver
- **Efficient color mapping** algorithms
- **30 FPS target** for smooth real-time simulation

## Troubleshooting

### Common Issues

1. **SFML not found**:
   ```bash
   # Ensure SFML is installed and linked correctly
   brew install sfml
   # Check SFML installation path
   brew --prefix sfml
   ```

2. **Build errors**:
   ```bash
   # Clean and rebuild
   make clean
   make
   ```

3. **Runtime errors**:
   - Ensure SFML libraries are in your library path
   - Check that the executable has proper permissions

### Debug Information
The simulation outputs debug information to the console:
- Initialization status
- Smoke values at inflow boundaries
- Build and window creation confirmations

## License

add MIT license later

## Acknowledgments

 Inspired by [Matthias Müller's 10-Minute Physics: Fluid Simulation](https://matthias-research.github.io/pages/tenMinutePhysics/17-fluidSim.pdf)
