#ifndef HITTABLE_H
#define HITTABLE_H

#include <memory>

#include "rtweekend.h"
#include "ray.h"

class material;

struct hit_record {
    point3 p;
    vec3 normal;
    std::shared_ptr<material> mat_ptr;
    double t;

    bool front_face;

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
    public:
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

class sphere : public hittable {
    public:
        sphere() {}
        sphere(point3 cen, double r, std::shared_ptr<material> m)
            : center(cen), radius(r), mat_ptr(m)
        {}

        bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
            vec3 oc = r.origin() - center;
            auto a = r.direction().length_squared();
            auto half_b = dot(oc, r.direction());
            auto c = oc.length_squared() - radius * radius;
            auto discriminant = half_b * half_b - a * c;
            if (discriminant < 0)
                return false;
            auto sqrtd = sqrt(discriminant);

            auto root = (-half_b - sqrtd) / a;
            if (root < t_min || t_max < root) {
                root = (-half_b + sqrtd) / a;
                if (root < t_min || t_max < root)
                    return false;
            }

            rec.t = root;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat_ptr = mat_ptr;
            return true;
        }

    public:
        point3 center;
        double radius;

    private:
        std::shared_ptr<material> mat_ptr;
};

#endif
