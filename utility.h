#pragma once

//Headers
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>

//Using statements
using std::make_shared;
using std::shared_ptr;

//Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

//Functions
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

//Random number in [0, 1)
inline double random_double() {
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

//More Headers
#include "vec3.h"
#include "color.h"
#include "interval.h"
#include "ray.h"

