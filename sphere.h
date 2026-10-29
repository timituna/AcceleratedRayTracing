#pragma once

class Sphere : public Hittable {
    public:

    Sphere(const Point3& center, double radius, shared_ptr<Material> mat) 
    : center(center), radius(std::fmax(0, radius)), mat(mat) {}

    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        Vec3 o2c = center - r.get_origin(); //Vector from ray origin to sphere center
        auto a = r.get_direction().lenght_squared();
        auto h = dot(r.get_direction(), o2c); //Let b = -2h = -2[d(C-Q)]
        auto c =  o2c.lenght_squared() - (radius * radius);
        auto discriminant = (h * h) - (a * c);
        
        if(discriminant < 0) return false;

        auto sqrt_d = std::sqrt(discriminant);
        auto root = (h - sqrt_d) / a;
        
        //Find the appropriate root that is within the range
        if (!ray_t.surrounds(root)) {
            root = (h + sqrt_d) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        hrec.t = root;
        hrec.p = r.get_point_at(root);
        Vec3 outward_normal = (hrec.p - center) / radius;
        hrec.set_face_normal(r, outward_normal);
        hrec.mat = mat;
        
        return true;
    }

    private:

    Point3 center;
    double radius;
    shared_ptr<Material> mat;
};
