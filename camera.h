#pragma once

#include "hittable.h"
#include "material.h"
#include "sphere.h"

#include <curand_kernel.h>

class Camera {
    public:

    float aspect_ratio = 1.0;
	int image_width = 720;
    int samples_per_pixel = 10; // Count of random samples for each pixel
    int max_depth = 10;
    float vfov = 90;
    Point3 look_from = Point3(0);
    Point3 look_at = Point3(0, 0, -1);
    Vec3 vup = Vec3(0, 1, 0);
    float defocus_angle = 0;
    float focus_dist = 10;

    __host__ __device__ Camera() {}

    __device__
    Color render(int i, int j, Vec3* fb, Hittable** world, curandState* rand_state) {
        Color pixel_color(0,0,0);
        for (int sample = 0; sample < samples_per_pixel; sample++) {
            Ray r = construct_ray(i, j, rand_state);
            pixel_color += ray_color(r, world, rand_state);
        }
        return pixel_color * pixel_samples_scale;
    };

    __host__
    int image_size() const {
        return image_width * image_height;
    }

    __host__ __device__
    int get_image_height() const {
        return image_height;
    }

    __host__
    void init() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = look_from;

        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);

        // Determine viewport dimensions.
        auto focal_length = (look_from - look_at).length();
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(look_from - look_at);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        auto viewport_lower_left =
            center - (focus_dist * w) - (viewport_u / 2) - (viewport_v / 2);
        pixel_00_loc = viewport_lower_left + 0.5 * (pixel_delta_u + pixel_delta_v);
        
        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

    }

    private:

    int image_height;
    Point3 center;
    Point3 pixel_00_loc;
    Vec3 pixel_delta_u;
    Vec3 pixel_delta_v;
    Vec3 u, v, w;
    float pixel_samples_scale;
    Vec3 defocus_disk_u;
    Vec3 defocus_disk_v;

    __device__
    Ray construct_ray(int i, int j, curandState* rand_state) {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.
        auto offset = sample_square(rand_state);
        auto pixel_sample = 
        pixel_00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample(rand_state);
        auto ray_direction = pixel_sample - ray_origin;

        return Ray(ray_origin, ray_direction);
    }

    __device__
    Vec3 sample_square(curandState* rand_state) const {
        return Vec3(curand_uniform(rand_state) - 0.5f, curand_uniform(rand_state) - 0.5f, 0);
    }

    __device__
    Vec3 defocus_disk_sample(curandState* rand_state) const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk(rand_state);
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    __device__
    Color ray_color(const Ray&r, Hittable** world, curandState* rand_state) const {
        Ray temp_r = r;
        Color cumulative_color = Color(1);
        
        for(int i = 0; i < max_depth; i++) {
            HitRecord hrec;

            if((*world)->hit(temp_r, Interval(0.001f, infinity), hrec)) {
                Ray scattered;
                Color attenuation;

                if(hrec.mat->scatter(temp_r, hrec, attenuation, scattered, rand_state)) {
                    cumulative_color = cumulative_color * attenuation;
                    temp_r = scattered;
                }
                else return Color(0);
            }
            else {
                Vec3 unit_dir = unit_vector(temp_r.get_direction());
                float a = 0.5f * (unit_dir.y() + 1.0f);
                return cumulative_color * ((1.0f - a) * Color(1.0) + a * Color(0.5, 0.7, 1.0));
            }
        }

        return Color(0);
    }
};
