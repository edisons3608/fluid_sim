#ifndef FLUID_H
#define FLUID_H

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
    Fluid(int width, int height, int gravity);
    
    // Destructor
    ~Fluid();
    
    // Methods
    void propagateGravity(float dt, float g);

    void applyIncompressibility(float dt, int tot_iter);

    void advect(float dt);
};

#endif // FLUID_H 