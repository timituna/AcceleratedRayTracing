#pragma once

#include "hittable.h"
#include "material.h"
#include "texture.h"

class ConstantMedium : public Hittable {
    public:

    ConstantMedium(shared_ptr<Hittable> boundary, double density, shared_ptr<Texture> tex) :
    boundary(boundary), neg_inv_density(-1 / density), phase_func(make_shared<Isotropic>(tex)) {}
    ConstantMedium(shared_ptr<Hittable> boundary, double density, const Color& albedo) :
    boundary(boundary), neg_inv_density(-1/density), phase_func(make_shared<Isotropic>(albedo)) {}
    
    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        HitRecord hrec1, hrec2;

        if (!boundary->hit(r, Interval::universe, hrec1))
            return false;
   
        if (!boundary->hit(r, Interval(hrec1.t+0.0001, infinity), hrec2))
            return false;

        
        if (hrec1.t < ray_t.min) hrec1.t = ray_t.min;
        if (hrec2.t > ray_t.max) hrec2.t = ray_t.max;
        
        if (hrec1.t >= hrec2.t)
            return false;
                
        if (hrec1.t < 0)
            hrec1.t = 0;

        auto ray_length = r.get_direction().length();
        auto distance_inside_bounds = (hrec2.t - hrec1.t) * ray_length;
        auto hit_distance = neg_inv_density * std::log(random_double());

        if (hit_distance > distance_inside_bounds)
            return false;
        
        hrec.t = hrec1.t + hit_distance / ray_length;
        hrec.p = r.get_point_at(hrec.t);
        hrec.n = Vec3(1, 0, 0); //arbitrary
        hrec.front_face = true; //arbitrary
        hrec.mat = phase_func;

        return true;
    }

    AABB get_bbox() const override { return boundary->get_bbox(); }

    private:

    shared_ptr<Hittable> boundary;
    double neg_inv_density;
    shared_ptr<Material> phase_func;
};
