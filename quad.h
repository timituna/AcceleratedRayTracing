#pragma once

#include "utility.h"
#include "hittable.h"
#include "hittable_list.h"

class Quad : public Hittable {
    public:

    Quad(const Point3& Q, const Vec3& u, const Vec3& v, shared_ptr<Material> mat) :
    Q(Q), u(u), v(v), mat(mat) {
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);

        set_bounding_box();
    }

    virtual void set_bounding_box() {
        // Compute the bounding box of all four vertices.
        auto bbox_diag_1 = AABB(Q, Q + u + v);
        auto bbox_diag_2 = AABB(Q + u, Q + v);
        bbox = AABB(bbox_diag_1, bbox_diag_2);
    }

    AABB get_bbox() const override { return bbox; }

    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        auto denom = dot(normal, r.get_direction());

        // No hit if the ray is parallel to the plane.
        if(std::fabs(denom) < 1e-8)
            return false;
        
        // Return false if the hit point parameter t is outside the ray interval.
        auto t = (D - dot(normal, r.get_origin())) / denom;
        if(!ray_t.contains(t))
            return false;
        
        auto intersection = r.get_point_at(t);
        Vec3 p_minus_q = intersection - Q;
        auto alpha = dot(w, cross(p_minus_q, v));
        auto beta = dot(w, cross(u, p_minus_q));

        if(!is_interior(alpha, beta, hrec))
            return false;
        
        hrec.t = t;
        hrec.p = intersection;
        hrec.mat = mat;
        hrec.set_face_normal(r, normal);

        return true;
    }

    virtual bool is_interior(double a, double b, HitRecord& hrec) const {
        Interval unit_interval = Interval(0, 1);
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.

        if(!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        hrec.u = a;
        hrec.v = b;
        return true;
    }

    private:

    Point3 Q;
    Vec3 u, v;
    Vec3 w;
    shared_ptr<Material> mat;
    AABB bbox;
    Vec3 normal;
    double D;
};

inline shared_ptr<HittableList> box(const Point3& a, const Point3& b, shared_ptr<Material> mat) {
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<HittableList>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = Point3(std::fmin(a.x(), b.x()), std::fmin(a.y(), b.y()), std::fmin(a.z(), b.z()));
    auto max = Point3(std::fmax(a.x(), b.x()), std::fmax(a.y(), b.y()), std::fmax(a.z(), b.z()));

    //GOES FROM min TO max
    auto dx = Vec3(max.x() - min.x(), 0, 0);
    auto dy = Vec3(0, max.y() - min.y(), 0);
    auto dz = Vec3(0, 0, max.z() - min.z());

    //Remmeber right hand rule - parallel surfaces should have opposite normals
    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()), dx, dz, mat)); //Bottom
    sides->add(make_shared<Quad>(Point3(min.x(), max.y(), max.z()), dx, -dz, mat)); //Top
    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), max.z()), dy, dx, mat)); //Back
    sides->add(make_shared<Quad>(Point3(max.x(), min.y(), min.z()), dy, -dx, mat)); //Front
    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()), dy, dz, mat)); //Left
    sides->add(make_shared<Quad>(Point3(max.x(), min.y(), max.z()), dy, -dz, mat)); //Right

    return sides;
}
