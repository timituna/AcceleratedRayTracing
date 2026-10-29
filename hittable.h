#pragma once

#include "aabb.h"

class Material;

class HitRecord {
    public:

    Point3 p; //point
    Vec3 n; //normal, in unit length
    shared_ptr<Material> mat; //material of the hit object
    double t; //intersection parameter
    double u; //Texcoord
    double v; //Texcoord
    bool front_face; //surface normal always points against the ray
    
    //outward_normal is assumed to be unit length
    void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face =  dot(r.get_direction(), outward_normal) < 0;
        n = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
    public:

    virtual ~Hittable() = default;
    virtual bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const = 0;
    virtual AABB get_bbox() const = 0;
};

class Translate : public Hittable {
    public:
    
    Translate(shared_ptr<Hittable> object, const Vec3& offset) : object(object), offset(offset) {
        bbox = object->get_bbox() + offset;
    }

    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        // Move the ray backwards by the offset
        Ray offset_r(r.get_origin() - offset, r.get_direction(), r.get_time());
    
        if(!object->hit(offset_r, ray_t, hrec))
            return false;
        
        hrec.p += offset;
        
        return true;
    }

    AABB get_bbox() const override { return bbox; }

    private:

    shared_ptr<Hittable> object;
    Vec3 offset;
    AABB bbox;
};

class RotateY : public Hittable {
    public:

    RotateY(shared_ptr<Hittable> object, double angle) : object(object) {
        auto radians = degrees_to_radians(angle);
        sin_theta = std::sin(radians);
        cos_theta = std::cos(radians);
        bbox = object->get_bbox();
        
        Point3 min( infinity,  infinity,  infinity);
        Point3 max(-infinity, -infinity, -infinity);
        
        for(int i = 0; i < 2; i++) {
            for(int j = 0; j < 2; j++) {
                for(int k = 0; k < 2; k++) {
                    auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
                    auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
                    auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

                    auto newx =  cos_theta * x + sin_theta * z;
                    auto newz = -sin_theta * x + cos_theta * z;

                    Point3 test(newx, y, newz);

                    for(int c = 0; c < 3; c++) {
                        min[c] = std::fmin(min[c], test[c]);
                        max[c] = std::fmax(max[c], test[c]);
                    }
                }
            }
        }

        bbox = AABB(min, max);
    }

    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        // Transform the ray from world space to object space.

        auto origin = Point3(
            (cos_theta * r.get_origin().x()) - (sin_theta * r.get_origin().z()),
            r.get_origin().y(),
            (sin_theta * r.get_origin().x()) + (cos_theta * r.get_origin().z())
        );

        auto direction = Vec3(
            (cos_theta * r.get_direction().x()) - (sin_theta * r.get_direction().z()),
            r.get_direction().y(),
            (sin_theta * r.get_direction().x()) + (cos_theta * r.get_direction().z())
        );

        Ray rotated_r(origin, direction, r.get_time());

        if(!object->hit(rotated_r, ray_t, hrec))
            return false;

        // Transform the intersection from object space back to world space.

        hrec.p = Point3(
            (cos_theta * hrec.p.x()) + (sin_theta * hrec.p.z()),
            hrec.p.y(),
            (-sin_theta * hrec.p.x()) + (cos_theta * hrec.p.z())
        );

        hrec.n = Vec3(
            (cos_theta * hrec.n.x()) + (sin_theta * hrec.n.z()),
            hrec.n.y(),
            (-sin_theta * hrec.n.x()) + (cos_theta * hrec.n.z())
        );

        return true;
    }

    AABB get_bbox() const override { return bbox; }

    private:

    shared_ptr<Hittable> object;
    double sin_theta;
    double cos_theta;
    AABB bbox;
};
