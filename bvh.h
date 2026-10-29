#pragma once

#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"
#include <algorithm>

class BVHNode : public Hittable {
    public:

    BVHNode(HittableList list) : BVHNode(list.objects, 0, list.objects.size()) {}

    BVHNode(std::vector<shared_ptr<Hittable>>& objects, size_t start, size_t end) {
        bbox = AABB::empty;
        for (size_t object_index = start; object_index < end; object_index++)
            bbox = AABB(bbox, objects[object_index]->get_bbox());

        int axis = bbox.longest_axis();


        auto comparator = (axis == 0) ? box_x_compare :
                          (axis == 1) ? box_y_compare : box_z_compare;
        
        size_t obj_span = end - start;

        if(obj_span == 1) {
            left = right = objects[start];
        }
        else if(obj_span == 2) {
            left = objects[start];
            right = objects[start + 1];
        }
        else{
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);
            auto mid = start + (obj_span / 2);
            left = make_shared<BVHNode>(objects, start, mid);
            right = make_shared<BVHNode>(objects, mid, end);
        }

        bbox = AABB(left->get_bbox(), right->get_bbox());
    }
    
    bool hit(const Ray& r, Interval ray_t, HitRecord& hrec) const override {
        if(!bbox.hit(r, ray_t)) return false;

        bool hit_left = left->hit(r, ray_t, hrec);
        bool hit_right = right->hit(r, Interval(ray_t.min, hit_left ? hrec.t : ray_t.max), hrec);
        
        return hit_left || hit_right;
    }

    AABB get_bbox() const override { return bbox; }
    
    private:

    shared_ptr<Hittable> left;
    shared_ptr<Hittable> right;
    AABB bbox;

    static bool box_compare(const shared_ptr<Hittable> a, 
                            const shared_ptr<Hittable> b, int axis_index) {
        auto a_axis_interval = a->get_bbox().axis_interval(axis_index);
        auto b_axis_interval = b->get_bbox().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) {
        return box_compare(a, b, 2);
    }
};
