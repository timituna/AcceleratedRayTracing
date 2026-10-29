#pragma once

#include "hittable.h"

class Material {
    public:

    virtual ~Material() = default;

    //r_out is the scattered ray
    virtual bool scatter([[maybe_unused]] const Ray& r_in, [[maybe_unused]] const HitRecord& hrec,
        [[maybe_unused]] Color& attenuation, [[maybe_unused]] Ray& r_out) const {
            return false;
        }
};

class Lambertian : public Material {
    public:

    Lambertian(const Color& albedo) : albedo(albedo) {}

    bool scatter([[maybe_unused]] const Ray& r_in, const HitRecord& hrec, Color& attenuation, Ray& r_out) 
    const override {
        auto scatter_dir = hrec.n + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_dir.near_zero())
            scatter_dir = hrec.n;

        r_out = Ray(hrec.p, scatter_dir);
        attenuation = albedo;
        return true;
    }

    private:

    Color albedo;
};

class Metal : public Material {
    public:

    Metal(const Color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz) {}

    bool scatter(const Ray& r_in, const HitRecord& hrec, Color& attenuation, Ray& r_out) 
    const override {
        Vec3 reflected = reflect(r_in.get_direction(), hrec.n);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        r_out = Ray(hrec.p, reflected);
        attenuation = albedo;
        return (dot(r_out.get_direction(), hrec.n) > 0);

    }

    private:
    
    Color albedo;
    double fuzz;
};

class Dielectric : public Material {
    public:
    
    Dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const Ray& r_in, const HitRecord& hrec, Color& attenuation, Ray& r_out)
    const override {
        attenuation = Color(1.0, 1.0, 1.0);
        double ri = hrec.front_face ? (1.0 / refraction_index) : refraction_index;
        Vec3 unit_dir = unit_vector(r_in.get_direction());
        double cos_theta = std::fmin(dot(-unit_dir, hrec.n), 1.0);
        double sin_theta = std::sqrt(1 - cos_theta * cos_theta);
        
        bool cannot_refract = ri * sin_theta > 1.0;
        Vec3 dir;

        if(cannot_refract || reflectance(cos_theta, ri) > random_double()) {
            dir = reflect(unit_dir, hrec.n);
        }
        else {
            dir = refract(unit_dir, hrec.n, ri);
        }

        r_out = Ray(hrec.p, dir);
        return true;
    }

    private:

    static double reflectance(double cos, double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cos), 5);
    }

    double refraction_index;
};
