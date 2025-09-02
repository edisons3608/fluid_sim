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
    float* m;  // Smoke field
    
    // Temporary arrays to avoid allocation in hot loops
    float* temp_u;
    float* temp_v;
    float* temp_f;
    float* temp_m;  // Temporary smoke field for advection
    
public:
    Fluid(int width, int height, float gravity, float density, float overrelax);
    ~Fluid();
    
    void propagateGravity(float dt, float g);
    void applyIncompressibility(float dt, int tot_iter);
    void extrapolate();
    float interpolateComponent(float x, float y, std::string vec_type);
    void advect(float dt);
    void advectSmoke(float dt);
    void simulate(float dt, int tot_iter, float g);
    
    float* getPressureField();
    float* getSmokeField();
    void setFluid(int i, int j, int value);
    void setSmoke(int i, int j, float value);
    void setU(int i, int j, float value);
    void setV(int i, int j, float value);
    void activateFluid();
    void resetPressure();

};

#endif // FLUID_H 