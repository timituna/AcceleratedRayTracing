#pragma once

class Material;

class HitRecord {
    public:

    Point3 p; //point
    Vec3 n; //normal, in unit length
    shared_ptr<Material> mat; //material of the hit object
    double t; //intersection parameter
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
    virtual bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const = 0;};
