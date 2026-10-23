#pragma once

#include "utility.h"

class Interval {
    public:

    float min;
    float max;

    __host__ __device__ Interval() : min(-infinity), max(infinity) {}
    __host__ __device__ Interval(float min, float max) : min(min), max(max) {}
    
    __host__ __device__
    float size() const {
        return max - min;
    }

    __host__ __device__
    bool contains(float x) const {
        return min <= x && x <= max;
    }

    __host__ __device__
    bool surrounds(float x) const {
        return min < x && x < max;
    }

    __host__ __device__
    float clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    static const Interval empty;
    static const Interval universe;
};

const Interval Interval::empty = Interval(infinity, -infinity);
const Interval Interval::universe = Interval(-infinity, infinity);
