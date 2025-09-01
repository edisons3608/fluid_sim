#include "Fluid.h"
#include <algorithm>

Fluid::Fluid(int width, int height, int gravity,float density,float overrelax) {
    // give buffer for easy calculation later
    this->width = width + 2;
    this->height = height + 2;
    this->gravity = gravity;
    this->density = density;
    this->totCells = this->width * this->height;
    this->overrelax = overrelax;
    this->u = new float[this->totCells];
    this->v = new float[this->totCells];
    this->s = new float[this->totCells];
    this->p = new float[this->totCells];
    this->h = 1.0;
}

Fluid::~Fluid() {
    delete[] this->u;
    delete[] this->v;
    delete[] this->s;
    delete[] this->p;
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

                this->v[i * stride + j] = this->v[i * stride + j] + g * dt;

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

                        this->u[i * stride + j] += p * s_left * this->overrelax;
                        this->u[(i+1) * stride + j] += p * s_right * this->overrelax;
                        this->v[i * stride + j] += p * s_down * this->overrelax;
                        this->v[i * stride + j+1] += p * s_up * this->overrelax;

                        this->p[i * stride + j] += p * this->overrelax*this->density*this->h/dt;

                    }

                    



                }

            }

        }

    }


}

void Fluid::advect(float dt){
    // use semi-lagrangian advection
    
    // Create local copies of velocity fields for reading while updating
    float* u_new = new float[this->totCells];
    float* v_new = new float[this->totCells];
    
    // Copy current velocity fields
    std::copy(this->u, this->u + this->totCells, u_new);
    std::copy(this->v, this->v + this->totCells, v_new);

    int stride = this->height;
    float half_cell = this->h/2;

    for(int i = 1; i < this->width - 1; i++){
        for(int j = 1; j < this->height - 1; j++){

            // u 
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

                //sampleField

                this->u[i * stride + j] = cur_u;

            

    


                
            }


                


        }
    }


    
    // Clean up temporary arrays
    delete[] u_new;
    delete[] v_new;
}