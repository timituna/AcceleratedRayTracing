#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <curand_kernel.h>

using std::make_shared;
using std::shared_ptr;

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

__host__ __device__
inline float degrees_to_radians(float degrees) {
    return degrees * pi / 180.0f;
}

__host__
inline float random_float() {
    // Returns a random real in [0,1).
    return std::rand() / (RAND_MAX + 1.0f);
}

__host__
inline float random_float(float min, float max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_float();
}

__device__
inline float d_random_float(curandState* rand_state) {
    return curand_uniform(rand_state);
}

__device__
inline float d_random_float(curandState* rand_state, float min, float max) {
    return min + (max - min) * curand_uniform(rand_state);
}

#include "vec3.h"
#include "ray.h"
#include "color.h"
#include "interval.h"
