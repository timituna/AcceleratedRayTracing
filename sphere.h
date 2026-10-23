#pragma once

#include "hittable.h"
#include "utility.h"

class Sphere : public Hittable {
    public:

    __device__ Sphere(const Point3& center, float radius, Material* mat) :
    center(center), radius(radius), mat(mat) {}
    
    __device__ ~Sphere() override { delete mat; }

    __device__ bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        Vec3 oc = center - r.get_origin();
        auto a = r.get_direction().length_squared();
        auto h = dot(r.get_direction(), oc);
        auto c = oc.length_squared() - radius * radius;
        auto discriminant = h * h - a * c;
        if (discriminant < 0)
            return false;

        auto sqrt_d = std::sqrt(discriminant);

        auto root = (h - sqrt_d) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrt_d) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        hrec.t = root;
        hrec.p = r.get_point_at(hrec.t);
        Vec3 outward_normal = (hrec.p - center) / radius;
        hrec.set_face_normal(r, outward_normal);
        hrec.mat = mat;

        return true;
    }

    private:

    Point3 center;
    float radius;
    Material* mat;
};
