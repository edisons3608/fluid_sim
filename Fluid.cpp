#include "Fluid.h"
#include <algorithm>
#include <string>

Fluid::Fluid(int width, int height, float gravity,float density,float overrelax) {
    // give buffer for easy calculation later
    this->width = width + 2;
    this->height = height + 2;
    this->gravity = gravity;
    this->density = density;
    this->totCells = this->width * this->height;
    this->overrelax = overrelax;
    this->u = new float[this->totCells];
    this->v = new float[this->totCells];
    this->s = new int[this->totCells];
    this->p = new float[this->totCells];
    this->m = new float[this->totCells];
    
    // Allocate temporary arrays to avoid allocation in hot loops
    this->temp_u = new float[this->totCells];
    this->temp_v = new float[this->totCells];
    this->temp_f = new float[this->totCells];
    this->temp_m = new float[this->totCells];
    
    // Initialize all fields to zero
    for(int i = 0; i < this->totCells; i++) {
        this->u[i] = 0.0f;
        this->v[i] = 0.0f;
        this->s[i] = 0;
        this->p[i] = 0.0f;
        this->m[i] = 0.0f;
    }
    
    this->h = 1.0;
}

Fluid::~Fluid() {
    delete[] this->u;
    delete[] this->v;
    delete[] this->s;
    delete[] this->p;
    delete[] this->m;
    delete[] this->temp_u;
    delete[] this->temp_v;
    delete[] this->temp_f;
    delete[] this->temp_m;
}

void Fluid::propagateGravity(float dt, float g) {
    //grid is defined where origin is at bottom left corner :)
    int stride = this->height;
    
    // can adjust later
    for(int i = 1; i < this->width; i++){

        // avoid update the boundary cell
        for(int j = 1; j < this->height-1; j++){
            // check cell above (i,j) and below (i,j-1) are free fluid.
            if (this->s[i * stride + j] != 0 && this->s[i * stride + j-1] != 0){

                this->v[i * stride + j] -= g * dt;

            }
        }
    }

}

void Fluid::applyIncompressibility(float dt, int tot_iter){
    int stride = this->height;

    //Gauss-Seidel method to solve incompressibility equations 
    
    for(int iter = 0;iter<tot_iter;iter++){
        for(int i = 1;i < this->width-1;i++){
            for(int j = 1;j < this->height-1;j++){

                // check that the current cell is free fluid and not solid or boundary
                float cur_s = this->s[i * stride + j];
                
                if(cur_s != 0){
                    // get the surrounding cells. Here, we use column major order for x index. Might change later...
                    float s_left = this->s[(i-1) * stride + j];
                    float s_right = this->s[(i+1) * stride + j];
                    float s_up = this->s[i * stride + (j+1)];
                    float s_down = this->s[i * stride + (j-1)];

                    float s_factor = s_left + s_right + s_up + s_down;

                    // s_facotr == 0  really should not happen in this code but just in case
                    if(s_factor != 0){
                        // get divergence of the velocity field at the current cell

                        // def outward flux as positive
                        float d = this->u[(i+1) * stride + j] - this->u[i * stride + j] + this->v[i * stride + (j+1)] - this->v[i * stride + j];

                        float p = -d / s_factor;

                        // solve based on poisson equation (but Gauss-Seidel method using iterated neighboring cells)

                        this->u[i * stride + j] -= p * s_left * this->overrelax;
                        this->u[(i+1) * stride + j] += p * s_right * this->overrelax;
                        this->v[i * stride + j] -= p * s_down * this->overrelax;
                        this->v[i * stride + j+1] += p * s_up * this->overrelax;

                        this->p[i * stride + j] += p * this->overrelax*this->density*this->h/dt;

                    }

                    



                }

            }

        }

    }


}

void Fluid::extrapolate() {
    int stride = this->height;
    
    // Extrapolate u field
    for (int i = 0; i < this->width; i++) {
        this->u[i * stride + 0] = this->u[i * stride + 1];
        this->u[i * stride + this->height - 1] = this->u[i * stride + this->height - 2];
    }
    
    // Extrapolate v field
    for (int j = 0; j < this->height; j++) {
        this->v[0 * stride + j] = this->v[1 * stride + j];
        this->v[(this->width - 1) * stride + j] = this->v[(this->width - 2) * stride + j];
    }
}

float Fluid::interpolateComponent(float x, float y, std::string vec_type){
    int stride = this->height;

    float half_cell = this->h/2;

    // bounding with ghost cells
    x = std::max(std::min(x, this->width * this->h), this->h);
    y = std::max(std::min(y, this->height * this->h), this->h);

    float dx = 0;
    float dy = 0;

    // Use pre-allocated temp array instead of allocating new memory
    float* f = this->temp_f;

    if(vec_type == "u"){
        std::copy(this->u, this->u + this->totCells, f);
        dy=half_cell;
    }
    else if(vec_type == "v"){
        std::copy(this->v, this->v + this->totCells, f);
        dx=half_cell;
    }
    else if(vec_type == "s"){
        std::copy(this->m, this->m + this->totCells, f);
        dx=half_cell;
        dy=half_cell;
    }
    else{
        return 0.0f;
    }

    // create bounding box for interpolation
    int x0 = std::min(static_cast<int>(std::floor((x - dx)/this->h)), this->width-1);
    int x1 = std::min(x0 + 1, this->width-1);

    int y0 = std::min(static_cast<int>(std::floor((y - dy)/this->h)), this->height-1); 
    int y1 = std::min(y0 + 1, this->height-1);

    float w_right = ((x - dx) - x0*this->h)/this->h;
    float w_up = ((y - dy) - y0*this->h)/this->h;

    float w_left = 1-w_right;
    float w_down = 1-w_up;

    float interpolated_value = w_left * w_down * f[x0 * stride + y0] + w_right * w_down * f[x1 * stride + y0] + w_right * w_up * f[x1 * stride + y1] + w_left * w_up * f[x0 * stride + y1];

    return interpolated_value;
}



void Fluid::advect(float dt){
    // use semi-lagrangian advection
    
    // Use pre-allocated temporary arrays instead of allocating new ones
    float* u_new = this->temp_u;
    float* v_new = this->temp_v;
    
    // Copy current velocity fields
    std::copy(this->u, this->u + this->totCells, u_new);
    std::copy(this->v, this->v + this->totCells, v_new);

    int stride = this->height;
    float half_cell = this->h/2;

    for(int i = 1; i < this->width - 1; i++){
        for(int j = 1; j < this->height - 1; j++){

            // u 

            //if above and below are boundary, don't advect
            if(this->s[i * stride + j] != 0 && this->s[(i-1) * stride + j] != 0){

                // universal loc of the u vector given (i,j)
                float x = i * this->h;
                float y = j * this->h + half_cell;

                float cur_u = this->u[i * stride + j];

                // get interpolated velocity surrounding the u vector
                float cur_v = (this->v[i * stride + j] + this->v[(i-1) * stride + j]+this->v[i * stride + (j+1)] + this->v[(i-1) * stride + (j+1)]) / 4;

                // linear parametric backtracking to find old pos
                x -= dt * cur_u;
                y -= dt * cur_v;

                cur_u = this->interpolateComponent(x,y,"u");

                u_new[i * stride + j] = cur_u;

                
            }
            // v 
            //if left and right are boundary, don't advect
            if(this->s[i * stride + j] != 0 && this->s[i * stride + (j-1)] != 0){

                // universal loc of the v vector given (i,j)
                float x = i * this->h + half_cell;
                float y = j * this->h;

                float cur_v = this->v[i * stride + j];

                // get interpolated velocity surrounding the v vector
                float cur_u = (this->u[i*stride + j-1] + this->u[i*stride+j] + this->u[(i+1)*stride+j-1] + this->u[(i+1)*stride+j])/4;

                // linear parametric backtracking to find old pos
                x -= dt * cur_u;
                y -= dt * cur_v;

                cur_v = this->interpolateComponent(x,y,"v");

                v_new[i * stride + j] = cur_v;

                
            }



                

        }
    }

    // copy the new velocity fields to the old velocity fields
    std::copy(u_new, u_new + this->totCells, this->u);
    std::copy(v_new, v_new + this->totCells, this->v);
}

void Fluid::advectSmoke(float dt) {
    // Use pre-allocated temporary array for smoke field
    float* m_new = this->temp_m;
    
    // Copy current smoke field
    std::copy(this->m, this->m + this->totCells, m_new);
    
    int stride = this->height;
    float h2 = 0.5f * this->h;
    
    for (int i = 1; i < this->width - 1; i++) {
        for (int j = 1; j < this->height - 1; j++) {
            if (this->s[i * stride + j] != 0) {
                // Get velocity at cell center by averaging neighboring velocity components
                //u stored at left of cell, v stored at bottom of cell
                float u = (this->u[i * stride + j] + this->u[(i+1) * stride + j]) * 0.5f;
                float v = (this->v[i * stride + j] + this->v[i * stride + j+1]) * 0.5f;
                
                // Calculate position to sample from (backtracking)
                float x = i * this->h + h2 - dt * u;
                float y = j * this->h + h2 - dt * v;
                
                // Sample smoke field at the backtracked position using interpolateComponent
                m_new[i * stride + j] = this->interpolateComponent(x, y, "s");
            }
        }
    }
    
    // Copy the new smoke field to the old smoke field
    std::copy(m_new, m_new + this->totCells, this->m);
}

void Fluid::simulate(float dt,int tot_iter,float g){

    this->propagateGravity(dt,g);
    this->resetPressure();
    this->applyIncompressibility(dt,tot_iter);
    this->extrapolate();
    this->advect(dt);
    this->advectSmoke(dt);

}

float* Fluid::getPressureField(){
    return this->p;
}

void Fluid::setFluid(int i, int j, int value){
    this->s[i * this->height + j] = value;
}

void Fluid::activateFluid(){
    for(int i = 0; i < this->width; i++){
        for(int j = 0; j < this->height; j++){
            this->s[i * this->height + j] = 1;
        }
    }

}

void Fluid::resetPressure(){
    for(int i = 0; i < this->width; i++){
        for(int j = 0; j < this->height; j++){
            this->p[i * this->height + j] = 0;
        }
    }
}

float* Fluid::getSmokeField(){
    return this->m;
}

void Fluid::setSmoke(int i, int j, float value){
    this->m[i * this->height + j] = value;
}

void Fluid::setU(int i, int j, float value){
    this->u[i * this->height + j] = value;
}

void Fluid::setV(int i, int j, float value){
    this->v[i * this->height + j] = value;
}