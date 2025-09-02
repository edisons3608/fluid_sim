#include "Fluid.h"
#include <algorithm>
#include <string>

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

float Fluid::interpolateComponent(float x, float y, std::string vec_type){
    int stride = this->height;

    float half_cell = this->h/2;

    // bounding with ghost cells
    x = std::max(std::min(x, this->width * this->h), this->h);
    y = std::max(std::min(y, this->height * this->h), this->h);

    float dx = 0;
    float dy = 0;

    // Create new float array of size totCells
    float* f = new float[this->totCells];

    if(vec_type == "u"){
        std::copy(this->u, this->u + this->totCells, f);
        dy=half_cell;
    }
    else if(vec_type == "v"){
        std::copy(this->v, this->v + this->totCells, f);
        dx=half_cell;
    }
    else if(vec_type == "s"){
        std::copy(this->s, this->s + this->totCells, f);
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
    float w_down = 1-w_down;

    float interpolated_value = w_left * w_down * f[x0 * stride + y0] + w_right * w_down * f[x1 * stride + y0] + w_right * w_up * f[x1 * stride + y1] + w_left * w_up * f[x0 * stride + y1];

    return interpolated_value;



    
    // Clean up the temp array
    delete[] f;
    
    return 0.0f; // Placeholder return value
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

                cur_u = this->interpolateComponent(x,y,"u");

                u_new[i * stride + j] = cur_u;

                
            }
            // v 
            if(this->s[i * stride + j] != 0 && this->s[(i-1) * stride + j] != 0){

                // universal loc of the u vector given (i,j)
                float x = i * this->h + half_cell;
                float y = j * this->h;

                float cur_v = this->v[i * stride + j];

                // get interpolated velocity surrounding the u vector
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


    
    // Clean up temporary arrays
    delete[] u_new;
    delete[] v_new;
}