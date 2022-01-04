#include <iostream>
#include <fstream>
#include <cmath>

#define SWAP(x0, x) {float * tmp = x0; x0 = x; x = tmp;}
#define IX(i, j) ((i) + (N+2) * (j))
#define PI_OVER_180 0.01745329252

const int N = 201;
const int frames = 240;
const int size = (N+2)*(N+2);
static float u[size], v[size], u_prev[size], v_prev[size];
static float dens[size], dens_prev[size];

void add_source(int N, float * x, float * s, float dt)  {
    int i, size = (N+2) * (N+2);
    for (i = 0; i < size; i++) x[i] += dt * s[i];
}

void set_bounds (int N, int b, float * x)   {
    for (int i = 1 ; i <= N ; i++)    {
        x[IX(0,  i)] = b==1 ? -x[IX(1,i)] : x[IX(1,i)]; // for closed box boundary
        x[IX(N+1,i)] = b==1 ? -x[IX(N,i)] : x[IX(N,i)]; 
        x[IX(i,  0)] = b==2 ? -x[IX(i,1)] : x[IX(i,1)]; 
        x[IX(i,N+1)] = b==2 ? -x[IX(i,N)] : x[IX(i,N)];

        /*x[IX(0,  i)] = x[IX(N,i)]; // for periodic boundary (b is redundant here)
        x[IX(N+1,i)] = x[IX(1,i)]; 
        x[IX(i,  0)] = x[IX(i,N)]; 
        x[IX(i,N+1)] = x[IX(i,1)];*/
    } 
    x[IX(0,    0)] = 0.5 * (x[IX(1,  0)] + x[IX(0,  1)]); 
    x[IX(0,  N+1)] = 0.5 * (x[IX(1,N+1)] + x[IX(0,  N)]); 
    x[IX(N+1,  0)] = 0.5 * (x[IX(N,  0)] + x[IX(N+1,1)]); 
    x[IX(N+1,N+1)] = 0.5 * (x[IX(N,N+1)] + x[IX(N+1,N)]); 
}

void diffuse(int N, int b, float * x, float * x0, float diff, float dt) {
    int i, j, k;
    float a = dt * diff * N * N;
    for (k = 0; k < 20; k++)    {
        for (i = 1; i <= N; i++)    {
            for (j = 1; j <= N; j++)    {
                x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(i-1, j)] + x[IX(i+1, j)] + x[IX(i, j-1)] + x[IX(i, j+1)])) / (1 + 4 * a);
            }
        }
        set_bounds(N, b, x);
    }
}

void advect(int N, int b, float * d, float * d0, float * u, float * v, float dt)    {
    int i, j, i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1, dt0;

    dt0 = dt * N;
    for (i = 1; i <=N; i++) {
        for (j = 1; j <=N; j++) {
            x = i - dt0 * u[IX(i, j)];
            y = j - dt0 * v[IX(i, j)];
            if (x < 0.5) x = 0.5;
            if (x > N + 0.5) x = N + 0.5;
            if (y < 0.5) y = 0.5;
            if (y > N + 0.5) y = N + 0.5;
            i0 = (int) x; j0 = (int) y;
            i1 = i0 + 1; j1 = j0 + 1;
            s1 = x - i0; s0 = 1 - s1; t1 = y - j0; t0 = 1 - t1;
            d[IX(i, j)] = s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) + s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
        }
    }
    set_bounds(N, b, d);
}

void project(int N, float * u, float * v, float * p, float * div)   {
    int i, j, k;
    float h;

    h = 1.0 / N;
    for (i = 1; i <= N; i++)    {
        for (j = 1; j <= N; j++)    {
            div[IX(i, j)] = -0.5 * h * (u[IX(i+1, j)] - u[IX(i-1, j)] + v[IX(i, j+1)] - v[IX(i, j-1)]);
            p[IX(i, j)] = 0; 
        }
    }
    set_bounds(N, 0, div); set_bounds(N, 0, p);

    for (k = 0; k < 20; k++)    {
        for (i = 1; i <= N; i++)    {
            for (j = 1; j <= N; j++)    {
                p[IX(i, j)] = (div[IX(i, j)] + p[IX(i-1, j)] + p[IX(i+1, j)] + p[IX(i, j-1)] + p[IX(i, j+1)]) / 4;
            }
        }
        set_bounds(N, 0, p);
    }

    for (i = 1; i <= N; i++)    {
        for (j = 1; j <= N; j++)    {
            u[IX(i, j)] -= 0.5 * (p[IX(i+1, j)] - p[IX(i-1, j)]) / h; 
            v[IX(i, j)] -= 0.5 * (p[IX(i, j+1)] - p[IX(i, j-1)]) / h; 
        }
    }
    set_bounds(N, 1, u); set_bounds(N, 2, v); 
}

void dens_step(int N, float * x, float * x0, float * u, float * v, float diff, float dt)    {
    SWAP (x0, x); diffuse(N, 0, x, x0, diff, dt);
    SWAP (x0, x); advect(N, 0, x, x0, u, v, dt);
}

void vel_step(int N, float * u, float * v, float * u0, float * v0, float visc, float dt)    {
    add_source(N, u, u0, dt); add_source(N, v, v0, dt);
    SWAP (u0, u); diffuse(N, 1, u, u0, visc, dt);
    SWAP (v0, v); diffuse(N, 2, v, v0, visc, dt);
    project(N, u, v, u0, v0);
    SWAP (u0, u); SWAP (v0, v);
    advect(N, 1, u, u0, u0, v0, dt); advect(N, 2, v, v0, u0, v0, dt);
    project(N, u, v, u0, v0);
}

int main()  {
    float visc = 0.00001;
    float dt = 0.1;
    float diff = 0.00001;
    float angle = (rand() % 360) * PI_OVER_180;
    float noise;
    int radius = N / 20;
    
    std::ofstream file;
    file.open("fluid_sim.dat");
    srand(time(NULL));
    file << N << "," << frames << "," << "\n";
    for (int i = 0; i < frames; i++)    {
        noise = (rand() % 30 - 15) * PI_OVER_180;
        angle += noise;
        
        for (int x = 0; x < N+2; x++)   {
            for (int y = 0; y < N+2; y++)   {
                if (((N-1)/2 - x)*((N-1)/2 - x) + ((N-1)/2 - y)*((N-1)/2 - y) < radius * radius)    {
                    dens[IX(x, y)] = 10;
                    u_prev[IX(x, y)] = cos(angle);
                    v_prev[IX(x, y)] = sin(angle);
                }
            }
        }
        
        vel_step(N, u, v, u_prev, v_prev, visc, dt);
        dens_step(N, dens, dens_prev, u, v, diff, dt);

        for (int x = 0; x < N+2; x++)  {
            for (int y = 0; y < N+2; y++)  {
                file << dens[IX(x, y)] << ",";
            }
        }
        file << "," << "\n";
    }
    file.close();
    return 0;
}
