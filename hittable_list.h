#pragma once

#include "utility.h"
#include "hittable.h"
#include <vector>

using std::make_shared;
using std::shared_ptr;

class HittableList : public Hittable {
    public:
    Hittable** list;
    int size;

    __device__ HittableList() {}
    __device__ HittableList(Hittable** list, int size) : list(list), size(size) {}
    
    __device__ bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        HitRecord temp_rec;
        bool hit_any = false;
        auto closest = ray_t.max;

        for(int i = 0; i < size; i++) {
            if(list[i]->hit(r, Interval(ray_t.min, closest), temp_rec)) {
                hit_any = true;
                closest = temp_rec.t;
                hrec = temp_rec;
            }
        }

        return hit_any;
    }

};
