#pragma once

class Ray{
    public:

    Ray() = default;
    Ray(const Point3& orig, const Vec3& dir) : 
    origin(orig), 
    direction(dir) {}

    const Point3& get_origin() const { return origin; }
    const Vec3& get_direction() const { return direction; }

    Point3 get_point_at(double t) const {
        return origin + direction * t;
    }

    private:

    Point3 origin;
    Vec3 direction;
};
