# Fluid Simulation

A 2D fluid simulation built with C++ and SFML, showing smoke visualization overlaid on a pressure field rendering.

# Preview
![Fluid Simulation Preview](assets/pre.gif)



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
- **Successive Over-Relaxation (SOR)**: 1.9 (for faster convergence)
- **Time step**: 1/60 second (per frame)
- **Iterations**: 20 pressure solver iterations per frame (for Gauss-Seidel)


## Technical Details

### Fluid Simulation Algorithm

The simulation implements a basic Eulerian fluid simulator. 

Assumptions
- The fluid is inviscid.
- The fluid is incompresible.

A staggered grid is applied to store horizontal (u) and vertical (v) velocity components. 

The process is as follows:

1. **Applying gravity condition** across all vertical components across a defined ```dt```.
   ![v_f = v_i - g*dt](https://latex.codecogs.com/svg.latex?v_f%20%3D%20v_i%20-%20g%5Ccdot%20dt)

2. **Projection for incompressibility** (systems are solved via Gauss-Seidel method with SOR acceleration).

   Here, outward flux is described as positive.
   ![omega = 1.9](https://latex.codecogs.com/svg.latex?%5Comega%20%3D%201.9)
   ![d = omega(u_{i+1,j}-u_{i,j}+v_{i,j+1}-v_{i,j})](https://latex.codecogs.com/svg.latex?d%20%3D%20%5Comega%28u_%7Bi%2B1%2Cj%7D-u_%7Bi%2Cj%7D%2Bv_%7Bi%2Cj%2B1%7D-v_%7Bi%2Cj%7D%29)

   To achieve incompressibiliy for each cell ($div=0$), the net divergence is equally in all non-boundary directions.

   The pressure projection is also calculated via
   ![p_f = p_i + (rho * h)/dt * (-d)/s](https://latex.codecogs.com/svg.latex?p_f%20%3D%20p_i%20&plus;%20%5Cfrac%7B%5Crho%20%5C%2C%20h%7D%7Bdt%7D%20%5Ccdot%20%5Cfrac%7B-d%7D%7Bs%7D)
   to approximate the Poisson equation for pressure
   ![nabla^2 p = (rho/Delta t) * nabla * u](https://latex.codecogs.com/svg.latex?%5Cnabla%5E2%20p%20%3D%20%5Cfrac%7B%5Crho%7D%7B%5CDelta%20t%7D%20%5C%2C%20%5Cnabla%20%5Ccdot%20u)

3. **Advection** (using semi-LaGrangian advection)
   Backtrace from the current face position (vector x represents any vector u or v)
   ![x_prev = x - Delta t * v(i,j)](https://latex.codecogs.com/svg.latex?x_%7B%5Ctext%7Bprev%7D%7D%20%3D%20x%20-%20%5CDelta%20t%20%5C%2C%20v%28i%2Cj%29)

   Bilinearly interpolate from the old field and transfer velocity component
   ![u^{t+Delta t}(x) = u^t(x_prev)](https://latex.codecogs.com/svg.latex?u%5E%7Bt%2B%5CDelta%20t%7D%28%5Cmathbf%7Bx%7D%29%20%3D%20u%5E%7Bt%7D%28%5Cmathbf%7Bx%7D_%7B%5Ctext%7Bprev%7D%7D%29)


   Smoke advection is utilized for visualization; the calculation is essentially the same as the velocity advection process.



## Acknowledgments

 Inspired by [Matthias Müller's 10-Minute Physics: Fluid Simulation](https://matthias-research.github.io/pages/tenMinutePhysics/17-fluidSim.pdf)
