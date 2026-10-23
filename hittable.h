#pragma once

#include "utility.h"

class Material;

class HitRecord {
    public:
    Point3 p;
    Vec3 n;
    float t;
    bool front_face;
    Material* mat;

    __device__ void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.get_direction(), outward_normal) < 0;
        n = front_face ? outward_normal : - outward_normal;
    }
};

class Hittable {
    public:
    __device__ virtual ~Hittable() = default;
    __device__ virtual bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const = 0;
};
