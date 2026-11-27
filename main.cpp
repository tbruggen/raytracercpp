#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "rtweekend.h"
#include "color.h"
#include "png.h"
#include "ray.h"
#include "hittable_list.h"
#include "material.h"


color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;

    if (depth <= 0)
        return color(0, 0, 0);

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);
}


hittable_list world_scene() {
    hittable_list world;

    auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = std::make_shared<lambertian>(color(0.7, 0.3, 0.3));
    auto material_left   = std::make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
    auto material_right  = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.8);

    world.add(std::make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
    world.add(std::make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(std::make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

    return world;
}


int main(int argc, char* argv[]) {

    std::string out_path = (argc > 1) ? argv[1] : "out.png";

        // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 50;

    // World

    auto world = world_scene();

    // Camera

    auto viewport_height = 2.0;
    auto viewport_width = aspect_ratio * viewport_height;
    auto focal_length = 1.0;

    auto origin = point3(0, 0, 0);
    auto horizontal = vec3(viewport_width, 0, 0);
    auto vertical = vec3(0, viewport_height, 0);
    auto lower_left_corner = origin - horizontal/2 - vertical/2 - vec3(0, 0, focal_length);

    // Render into a pixel buffer, top row first

    std::vector<color> pixels(image_width * image_height);

    for (int j = image_height-1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r(origin, lower_left_corner + u*horizontal + v*vertical - origin);
                pixel_color += ray_color(r, world, max_depth);
            }
            pixels[(image_height-1-j)*image_width + i] = pixel_color;
        }
    }

    // Write the image file

    if (out_path.size() >= 4 && out_path.compare(out_path.size()-4, 4, ".ppm") == 0) {
        std::ofstream out(out_path);
        out << "P3\n" << image_width << " " << image_height << "\n255\n";
        for (const auto& p : pixels)
            write_color(out, p, samples_per_pixel);
    } else {
        if (!write_png(out_path, image_width, image_height, pixels, samples_per_pixel)) {
            std::cerr << "\nFailed to write " << out_path << "\n";
            return 1;
        }
    }

    std::cerr << "\nWrote " << out_path << " (" << image_width << "x" << image_height << ")\n";
}
