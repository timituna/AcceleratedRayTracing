#pragma once

#include "vec3.h"

class Ray {
    public:

    __device__ Ray() {}
    __device__ Ray(const Point3& orig, const Vec3 dir) : orig(orig), dir(dir) {}

    __device__ const Point3 get_origin() const { return orig; }
    __device__ const Vec3 get_direction() const { return dir; }

    __device__ Point3 get_point_at(float t) const {
        return orig + dir * t;
    }
    
    private:

    Point3 orig;
    Vec3 dir;
};
