#pragma once

#include "perlin.h"
#include "rtw_image.h"

class Texture {
    public: 
    virtual ~Texture() = default;
    virtual Color value(double u, double v, const Point3& p) const = 0;
};

class SolidColor : public Texture {
    public:
    
    SolidColor(const Color& albedo) : albedo(albedo) {}
    SolidColor(double r, double g, double b) : SolidColor(Color(r, g, b)) {}

    Color value([[maybe_unused]] double u, [[maybe_unused]] double v, 
                [[maybe_unused]] const Point3& p) const override {
        return albedo;
    }

    private:

    Color albedo;
};

class CheckerTexture : public Texture {
    public:

    CheckerTexture(double scale, shared_ptr<Texture> even, shared_ptr<Texture> odd)
    : inv_scale(1.0 / scale), even(even), odd(odd) {}
    CheckerTexture(double scale, const Color& c1, const Color& c2)
    : CheckerTexture(scale, make_shared<SolidColor>(c1), make_shared<SolidColor>(c2)) {}

    Color value(double u, double v, const Point3& p) const override {
        auto xInt = int(std::floor(inv_scale * p.x()));
        auto yInt = int(std::floor(inv_scale * p.y()));
        auto zInt = int(std::floor(inv_scale * p.z()));

        bool isEven = (xInt + yInt + zInt) % 2 == 0;
        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

    private:

    double inv_scale;
    shared_ptr<Texture> even;
    shared_ptr<Texture> odd;
};

class ImageTexture : public Texture {
    public:

    ImageTexture(const char* image) : image(image) {}

    Color value(double u, double v, [[maybe_unused]] const Point3& p) const override {
        // If we have no texture data, then return solid cyan as a debugging aid.
        if (image.height() <= 0) return Color(0,1,1);

        // Clamp input texture coordinates to [0,1] x [1,0]
        u = Interval(0, 1).clamp(u);
        v = 1.0 - Interval(0, 1).clamp(v);  // Flip V to image coordinates

        auto i = int(u * image.width());
        auto j = int(v * image.height());
        auto pixel = image.pixel_data(i, j);

        auto color_scale = 1.0 / 255.0;
        return Color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
    }
    
    private:
    
    RTWImage image;
};

class NoiseTexture : public Texture {
    public:
    NoiseTexture(double scale) : scale(scale) {}

    Color value([[maybe_unused]] double u, [[maybe_unused]] double v, const Point3& p) const override {
        return Color(1,1,1) * 0.5 * (1.0 + p_noise.noise(scale * p));
        //return Color(1, 1, 1) * p_noise.turb(p, 7); //turbulance
        //return Color(.5, .5, .5) * (1 + std::sin(scale * p.z() + 10 * p_noise.turb(p, 7))); //Phase adjusted
    }
    
    private:
    Perlin p_noise;
    double scale;
};
