#ifndef FLUID_H
#define FLUID_H

#include <string>

class Fluid {
private:
    int width;
    int height;
    int totCells;
    float gravity;
    float density;
    float overrelax;
    float h;
    
    float* u;
    float* v;
    int* s;
    float* p;
    
    // Temporary arrays to avoid allocation in hot loops
    float* temp_u;
    float* temp_v;
    float* temp_f;
    
public:
    Fluid(int width, int height, float gravity, float density, float overrelax);
    ~Fluid();
    
    void propagateGravity(float dt, float g);
    void applyIncompressibility(float dt, int tot_iter);
    void extrapolate();
    float interpolateComponent(float x, float y, std::string vec_type);
    void advect(float dt);
    void simulate(float dt, int tot_iter, float g);
    
    float* getPressureField();
    void setFluid(int i, int j, int value);
    void activateFluid();
    void resetPressure();
};

#endif // FLUID_H 