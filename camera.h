#pragma once

#include "hittable.h"
#include "material.h"

class camera {
    public:
    double aspect_ratio = 16.0 / 10.0;  // Ratio of image width over height
    int image_width  = 720;  // Rendered image width in pixel count
    int rndm_smpls_pxl = 10; //Random samples for each pixel
    int grid_size = 2; //Not the actual grid size but the square root of it
    int max_depth = 10;   // Maximum number of ray bounces into scene
    double vfov = 90; //vertical field of view
    Point3 lookfrom = Point3(0,0,0);   // Point camera is looking from
    Point3 lookat = Point3(0,0,-1);  // Point camera is looking at
    Vec3 vup = Vec3(0,1,0);     // Camera-relative "up" direction
    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    void render(const Hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                Color pixel_color(0, 0, 0);

                for(int sample = 0; sample < rndm_smpls_pxl; sample++) {
                    Ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                write_color(std::cout, pxl_smpls_scale * pixel_color);

                //write_color(std::cout, pxl_grid_scale * grid_sample_ray(i, j, world));
            }
        }

        std::clog << "\rDone.                 \n";
    }

    private:
    int image_height;   // Rendered image height
    double pxl_smpls_scale;
    double pxl_grid_scale;
    Point3 center;         // Camera center
    Point3 pixel_00_center;    // Location of pixel 0, 0
    Vec3 pixel_delta_u;  // Offset to pixel to the right
    Vec3 pixel_delta_v;  // Offset to pixel below
    Vec3 u, v, w; // Camera frame basis vectors
    Vec3 defocus_disk_u; // Defocus disk horizontal radius
    Vec3 defocus_disk_v; // Defocus disk vertical radius

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        
        pxl_smpls_scale = 1.0 / rndm_smpls_pxl;
        pxl_grid_scale = 1.0 / (grid_size * grid_size);
        center = lookfrom;

        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / image_height);
        
        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);
        
        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;

        pixel_00_center = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
        
        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    Ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.

        auto offset = random_sample_square();
        auto pixel_sample = pixel_00_center
                          + (i * pixel_delta_u + pixel_delta_u * offset.x())
                          + (j * pixel_delta_v + pixel_delta_v * offset.y());
        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_dir = pixel_sample - ray_origin;

        return Ray(ray_origin, ray_dir);
    }

    Vec3 random_sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return Vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }


    Point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    Color grid_sample_ray(int px_i, int px_j, int depth, const Hittable& world) const {
        //px_i and px_j represent pixel coordinates
        //i and j represent grid coords, each pixel has a grid

        Color pixel_color(0, 0, 0);

        for(double i = 0; i < grid_size; i++) {
            for(double j = 0; j < grid_size; j++) {
                Vec3 offset((i + 0.5) / grid_size, (j + 0.5) / grid_size, 0.0);
                auto pixel_sample = pixel_00_center
                    + (px_i * pixel_delta_u + pixel_delta_u * offset.x())
                    + (px_j * pixel_delta_v + pixel_delta_v * offset.y());
                auto ray_origin = center;
                auto ray_dir = pixel_sample - ray_origin;
                Ray r(ray_origin, ray_dir);
                pixel_color += ray_color(r, depth - 1, world);
            }
        }
        
        return Color(pixel_color.x(), pixel_color.y(), pixel_color.z());
    }

    Color ray_color(const Ray& r, int depth, const Hittable& world) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return Color(0,0,0);

        HitRecord hrec;

        if (world.hit(r, Interval(0.0001, infinity), hrec)) {
            Ray scattered;
            Color attenuation;
            if(hrec.mat->scatter(r, hrec, attenuation, scattered)) {
                return attenuation * ray_color(scattered, depth - 1, world);
            }
            return Color(0, 0, 0);
        }

        Vec3 unit_direction = unit_vector(r.get_direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0 - a) * Color(1.0, 1.0, 1.0) + a * Color(0.5, 0.7, 1.0);
    }
};
