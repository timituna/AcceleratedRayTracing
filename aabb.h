#pragma once

class AABB{
    public:

    Interval x, y, z;

    AABB() = default;
    AABB(const Interval& x, const Interval& y, const Interval& z) : x(x), y(y), z(z) {
        pad_to_minimum();
    }
    AABB(const Point3& a, const Point3& b) {
        x = (a[0] <= b[0]) ? Interval(a[0], b[0]) : Interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? Interval(a[1], b[1]) : Interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? Interval(a[2], b[2]) : Interval(b[2], a[2]);

        pad_to_minimum();
    }
    AABB(const AABB& bbox1, const AABB& bbox2) {
        x = Interval(bbox1.x, bbox2.x);
        y = Interval(bbox1.y, bbox2.y);
        z = Interval(bbox1.z, bbox2.z);
    }
    
    const Interval& axis_interval(int n) const {
        if(n == 1) return y;
        else if(n == 2) return z;
        else return x;
    }

    bool hit(const Ray& r, Interval ray_t) const {
        const Point3& ray_origin = r.get_origin();
        const Vec3& ray_dir = r.get_direction();

        for(int axis = 0; axis < 3; axis++) {
            const Interval& ax = axis_interval(axis);
            const double ad_inv = 1.0 / ray_dir[axis];
            
            auto t0 = (ax.min - ray_origin[axis]) * ad_inv;
            auto t1 = (ax.max - ray_origin[axis]) * ad_inv;

            if(t0 < t1) {
                if(t0 > ray_t.min) ray_t.min = t0;
                if(t1 < ray_t.max) ray_t.max = t1;
            }
            else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min) return false;
            
        }

        return true;
    }

    int longest_axis() const {
        if(x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;
    }

    static const AABB empty, universe;

    private:
    
    void pad_to_minimum() {
        // Adjust the AABB so that no side is narrower than some delta, padding if necessary.

        double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

const AABB AABB::empty = AABB(Interval::empty, Interval::empty, Interval::empty);
const AABB AABB::universe = AABB(Interval::universe, Interval::universe, Interval::universe);

AABB operator+(const AABB& bbox, const Vec3& offset) {
    return AABB(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

AABB operator+(const Vec3& offset, const AABB& bbox) {
    return bbox + offset;
}
