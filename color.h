#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "rtweekend.h"

#include <iostream>

void write_color(std::ostream &out, color pixel_color, int samples_per_pixel) {
    // Write the translated [0,255] value of each color component.
    auto scale = 1.0 / samples_per_pixel;
    auto r = scale * pixel_color.x();
    auto g = scale * pixel_color.y();
    auto b = scale * pixel_color.z();

    out << static_cast<int>(255.999 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(255.999 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(255.999 * clamp(b, 0.0, 0.999)) << '\n';
}

#endif