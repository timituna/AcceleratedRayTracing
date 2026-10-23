#pragma once

#include "utility.h"

class Vec3 {
  public:
    float e[3];

    __host__ __device__ Vec3() : e{0, 0, 0} {}
    __host__ __device__ Vec3(float e0, float e1, float e2) : e{e0, e1, e2} {}
    __host__ __device__ Vec3(float e) : e{e, e, e} {}

    __host__ __device__ float x() const { return e[0]; }
    __host__ __device__ float y() const { return e[1]; }
    __host__ __device__ float z() const { return e[2]; }

    __host__ __device__ Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }
    __host__ __device__ float operator[](int i) const { return e[i]; }
    __host__ __device__ float& operator[](int i) { return e[i]; }

    __host__ __device__ 
    Vec3& operator+=(const Vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    __host__ __device__
    Vec3& operator*=(float t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    __host__ __device__
    Vec3& operator/=(float t) {
        return *this *= 1/t;
    }

    __host__ __device__
    float length() const {
        return std::sqrt(length_squared());
    }

    __host__ __device__
    float length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    __host__ __device__
    bool near_zero() const {
        const auto s = 1e-8f;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }

    __device__
    static Vec3 random(curandState* rand_state) {
        return Vec3(d_random_float(rand_state),
                    d_random_float(rand_state),
                    d_random_float(rand_state));
    }

    __device__
    static Vec3 random(curandState* rand_state, float min, float max) {
        return Vec3(d_random_float(rand_state, min, max),
                    d_random_float(rand_state, min, max),
                    d_random_float(rand_state, min, max));
    }
};

// point3 is just an alias for Vec3
using Point3 = Vec3;

// Vector Utility Functions
inline std::ostream& operator<<(std::ostream& out, const Vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

__host__ __device__ inline Vec3 operator+(const Vec3& u, const Vec3& v) {
    return Vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

__host__ __device__ inline Vec3 operator-(const Vec3& u, const Vec3& v) {
    return Vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

__host__ __device__ inline Vec3 operator*(const Vec3& u, const Vec3& v) {
    return Vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

__host__ __device__ inline Vec3 operator*(float t, const Vec3& v) {
    return Vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

__host__ __device__ inline Vec3 operator*(const Vec3& v, float t) {
    return t * v;
}

__host__ __device__ inline Vec3 operator/(const Vec3& v, float t) {
    return (1/t) * v;
}

__host__ __device__ inline float dot(const Vec3& u, const Vec3& v) {
    return u.e[0] * v.e[0]
         + u.e[1] * v.e[1]
         + u.e[2] * v.e[2];
}

__host__ __device__ inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

__host__ __device__ inline Vec3 unit_vector(const Vec3& v) {
    return v / v.length();
}

__device__
inline Vec3 random_in_unit_disk(curandState* rand_state) {
    while (true) {
        auto p = Vec3(d_random_float(rand_state, -1, 1), d_random_float(rand_state, -1, 1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

__device__
inline Vec3 random_unit_vector(curandState* rand_state) {
    while(true) {
        auto p = Vec3::random(rand_state, -1, 1);
        auto len_sq = p.length_squared();
        if(1e-20f < len_sq && len_sq <= 1) {
            return p / sqrt(len_sq);
        }
    }
}

__device__
inline Vec3 random_on_hemisphere(const Vec3& normal, curandState* rand_state) {
    Vec3 on_unit_sphere = random_unit_vector(rand_state);
    if (dot(on_unit_sphere, normal) > 0.0f) // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

__device__
inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2 * dot(v, n) * n;
}

__device__
inline Vec3 refract(const Vec3& uv, const Vec3& n, float eta_i_over_eta_t) {
    auto cos_theta = fmin(dot(-uv, n), 1.0f);
    Vec3 r_out_perp =  eta_i_over_eta_t * (uv + cos_theta * n);
    Vec3 r_out_parallel = -sqrt(fabs(1.0f - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}
