#ifndef FLUID_H
#define FLUID_H

#include <string>

class Fluid {
private:
    // Add your private members here
    int width;
    int height;
    float gravity;
    int totCells;
    float overrelax;
    float density;
    float h;
    float* u;  // velocity field x-component
    float* v;  // velocity field y-component
    float* s;  // scalar field
    float* p;  // pressure field

public:
    // Constructor
    Fluid(int width, int height, float gravity, float density, float overrelax);
    
    // Destructor
    ~Fluid();
    
    // Methods
    void propagateGravity(float dt, float g);

    void applyIncompressibility(float dt, int tot_iter);

    void advect(float dt);
    
    float interpolateComponent(float x, float y, std::string vec_type);
    
    void simulate(float dt, int tot_iter, float g);
    
    float* getPressureField();
};

#endif // FLUID_H 