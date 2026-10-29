#pragma once

#include "hittable.h"
#include "aabb.h"
#include "texture.h"

class Material {
    public:

    virtual ~Material() = default;

    //r_out is the scattered ray
    virtual bool scatter([[maybe_unused]] const Ray& r_in, [[maybe_unused]] const HitRecord& hrec,
        [[maybe_unused]] Color& attenuation, [[maybe_unused]] Ray& r_out) const {
            return false;
        }
    virtual Color emitted([[maybe_unused]] double u, [[maybe_unused]] double v, 
        [[maybe_unused]] const Point3& p) const {
        return Color(0);
    }
};

class Lambertian : public Material {
    public:

    Lambertian(const Color& albedo) : tex(make_shared<SolidColor>(albedo)) {}
    Lambertian(shared_ptr<Texture> tex) : tex(tex) {}

    bool scatter([[maybe_unused]] const Ray& r_in, const HitRecord& hrec, Color& attenuation, Ray& r_out) 
    const override {
        auto scatter_dir = hrec.n + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_dir.near_zero())
            scatter_dir = hrec.n;

        r_out = Ray(hrec.p, scatter_dir, r_in.get_time());
        attenuation = tex->value(hrec.u, hrec.v, hrec.p);
        return true;
    }

    private:

    shared_ptr<Texture> tex;
};

class Metal : public Material {
    public:

    Metal(const Color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz) {}

    bool scatter(const Ray& r_in, const HitRecord& hrec, Color& attenuation, Ray& r_out) 
    const override {
        Vec3 reflected = reflect(r_in.get_direction(), hrec.n);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        r_out = Ray(hrec.p, reflected, r_in.get_time());
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

        r_out = Ray(hrec.p, dir, r_in.get_time());
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

class DiffuseLight : public Material {
    public:

    DiffuseLight(shared_ptr<Texture> tex) : tex(tex) {}
    DiffuseLight(const Color& emit) : tex(make_shared<SolidColor>(emit)) {}

    Color emitted(double u, double v, const Point3& p) const override {
        return tex->value(u, v, p);
    }

    private:
    
    shared_ptr<Texture> tex;
};

class Isotropic : public Material {
    public:

    Isotropic(const Color& albedo) : tex(make_shared<SolidColor>(albedo)) {}
    Isotropic(shared_ptr<Texture> tex) : tex(tex) {}

    bool scatter(const Ray& r_in, const HitRecord& hrec, Color& attenuation, Ray& scattered) 
    const override {
        scattered = Ray(hrec.p, random_unit_vector(), r_in.get_time());
        attenuation = tex->value(hrec.u, hrec.v, hrec.p);
        return true;
    }

    private:

    shared_ptr<Texture> tex;
};
