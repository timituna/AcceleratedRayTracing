#pragma once

class Sphere : public Hittable {
    public:
    
    // Stationary
    Sphere(const Point3& center, double radius, shared_ptr<Material> mat) 
    : center(center, Vec3(0, 0, 0)), radius(std::fmax(0, radius)), mat(mat) {
        auto rvec = Vec3(radius, radius, radius);
        bbox = AABB(center - rvec, center + rvec);
    }

    //Moving
    Sphere(const Point3& center0, const Point3& center1, double radius, shared_ptr<Material> mat)
    : center(center0, center1 - center0), radius(radius), mat(mat) {
        auto rvec = Vec3(radius, radius, radius);
        auto bbox1 = AABB(center.get_point_at(0) - rvec, center.get_point_at(0) + rvec);
        auto bbox2 = AABB(center.get_point_at(1) - rvec, center.get_point_at(1) + rvec);
        bbox = AABB(bbox1, bbox2);
    }

    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        Point3 current_center = center.get_point_at(r.get_time());
        Vec3 o2c = current_center - r.get_origin(); //Vector from ray origin to sphere center
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
        Vec3 outward_normal = (hrec.p - current_center) / radius;
        hrec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, hrec.u, hrec.v);
        hrec.mat = mat;
        
        return true;
    }

    AABB get_bbox() const override { return bbox; }

    private:

    static void get_sphere_uv(const Point3& p, double& u, double& v) {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
        auto theta = std::acos(-p.y());
        auto phi = std::atan2(-p.x(), p.z()) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }

    Ray center;
    double radius;
    shared_ptr<Material> mat;
    AABB bbox;
};
