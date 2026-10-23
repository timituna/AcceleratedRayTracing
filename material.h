#pragma once

#include "hittable.h"

class Material {
    public:

    __device__ 
    virtual ~Material() = default;

    __device__ 
    virtual bool scatter(const Ray& r_in, const HitRecord& hrec, 
        Color& attenuation, Ray& scattered, curandState* rand_state) const {
        
        return false;
    }
};

class Lambertian : public Material {
    public:

    __device__ 
    Lambertian(const Color& albedo) : albedo(albedo) {}

    __device__ 
    bool scatter(const Ray& r_in, const HitRecord& hrec, 
        Color& attenuation, Ray& scattered, curandState* rand_state) const override {
        auto scatter_dir = hrec.n + random_unit_vector(rand_state);

        if (scatter_dir.near_zero())
            scatter_dir = hrec.n;

        scattered = Ray(hrec.p, scatter_dir);
        attenuation = albedo;
        return true;
    }

    private:

    Color albedo;
};

class Metal : public Material {
    public:

    __device__ 
    Metal(const Color& albedo, float fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    __device__ 
    bool scatter(const Ray& r_in, const HitRecord& hrec,
        Color& attenuation, Ray& scattered, [[maybe_unused]] curandState* rand_state) const override {
            auto reflected = reflect(r_in.get_direction(), hrec.n);
            reflected = unit_vector(reflected) + (fuzz * random_unit_vector(rand_state));
            scattered = Ray(hrec.p, reflected);
            attenuation = albedo;
            return (dot(scattered.get_direction(), hrec.n) > 0);
        }

    private:

    Color albedo;
    float fuzz;
};

class Dielectric : public Material {
    public:

    __device__
    Dielectric(float r_idx) : refraction_index(r_idx) {}
    //Dielectric(const Color& albedo, float r_idx) : albedo(albedo), refraction_index(r_idx) {}

    __device__
    bool scatter(const Ray& r_in, const HitRecord& hrec,
        Color& attenuation, Ray& scattered, curandState* rand_state) const override {
            attenuation = Color(1);
            float ri = hrec.front_face ? (1.0f / refraction_index) : refraction_index;
            Vec3 unit_dir = unit_vector(r_in.get_direction());
            float cos_theta = fmin(dot(-unit_dir, hrec.n), 1.0f);
            float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
            bool cannot_refract = ri * sin_theta > 1.0f;
            Vec3 dir;
            
            if(cannot_refract || reflectance(cos_theta, ri) > d_random_float(rand_state))
                dir = reflect(unit_dir, hrec.n);
            else
                dir = refract(unit_dir, hrec.n, ri);

            scattered = Ray(hrec.p, dir);
            return true;
        }

    private:
    //Color albedo;
    float refraction_index;
    
    __device__
    static float reflectance(float cosine, float refraction_index) {
    // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        float m = 1 - cosine;
        float m2 = m * m;
        return r0 + (1 - r0) * (m2 * m2 * m);
    }
};
