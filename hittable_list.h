#pragma once

#include <vector>

using std::vector;

class HittableList : public Hittable {
    public:

    vector<shared_ptr<Hittable>> objects;

    HittableList() = default;
    HittableList(shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<Hittable> object) { 
        objects.push_back(object); 
        bbox = AABB(bbox, object->get_bbox());
    }

    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        HitRecord temp_rec;
        bool hit = false;
        auto closest = ray_t.max;

        for(const auto& object : objects) {
            if(object->hit(r, Interval(ray_t.min, closest), temp_rec)) {
                hit = true;
                closest = temp_rec.t;
                hrec = temp_rec;
            }
        }

        return hit;
    }

    AABB get_bbox() const override { return bbox; }

    private:

    AABB bbox;
};
