#ifndef PNG_H
#define PNG_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "rtweekend.h"
#include "vec3.h"

namespace png_detail {

inline void append_be32(std::vector<unsigned char>& out, uint32_t v) {
    out.push_back((unsigned char)((v >> 24) & 0xFF));
    out.push_back((unsigned char)((v >> 16) & 0xFF));
    out.push_back((unsigned char)((v >> 8) & 0xFF));
    out.push_back((unsigned char)(v & 0xFF));
}

inline uint32_t crc32(const unsigned char* data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int k = 0; k < 8; ++k)
            crc = (crc & 1u) ? (crc >> 1) ^ 0xEDB88320u : (crc >> 1);
    }
    return crc ^ 0xFFFFFFFFu;
}

inline uint32_t adler32(const unsigned char* data, size_t len) {
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < len; ++i) {
        a = (a + data[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

inline void append_chunk(std::vector<unsigned char>& png, const char* type, const std::vector<unsigned char>& data) {
    append_be32(png, (uint32_t)data.size());
    size_t type_pos = png.size();
    png.insert(png.end(), type, type + 4);
    png.insert(png.end(), data.begin(), data.end());
    append_be32(png, crc32(png.data() + type_pos, 4 + data.size()));
}

inline void append_stored_zlib(std::vector<unsigned char>& out, const unsigned char* data, size_t len) {
    out.push_back(0x78);
    out.push_back(0x01);
    size_t pos = 0;
    do {
        size_t n = len - pos;
        if (n > 65535) n = 65535;
        bool is_final = (pos + n == len);
        out.push_back((unsigned char)(is_final ? 0x01 : 0x00));
        out.push_back((unsigned char)(n & 0xFF));
        out.push_back((unsigned char)((n >> 8) & 0xFF));
        out.push_back((unsigned char)(~n & 0xFF));
        out.push_back((unsigned char)((~n >> 8) & 0xFF));
        out.insert(out.end(), data + pos, data + pos + n);
        pos += n;
    } while (pos < len);
    append_be32(out, adler32(data, len));
}

} // namespace png_detail

inline bool write_png(const std::string& path, int width, int height, const std::vector<color>& pixels, int samples_per_pixel) {
    if ((int)pixels.size() != width * height)
        return false;

    double scale = 1.0 / samples_per_pixel;
    std::vector<unsigned char> raw;
    raw.reserve((size_t)(width * 3 + 1) * height);
    for (int j = 0; j < height; ++j) {
        raw.push_back(0); // filter type: none
        for (int i = 0; i < width; ++i) {
            const color& c = pixels[j * width + i];
            raw.push_back((unsigned char)(255.999 * clamp(scale * c.x(), 0.0, 0.999)));
            raw.push_back((unsigned char)(255.999 * clamp(scale * c.y(), 0.0, 0.999)));
            raw.push_back((unsigned char)(255.999 * clamp(scale * c.z(), 0.0, 0.999)));
        }
    }

    std::vector<unsigned char> idat;
    png_detail::append_stored_zlib(idat, raw.data(), raw.size());

    std::vector<unsigned char> ihdr;
    png_detail::append_be32(ihdr, (uint32_t)width);
    png_detail::append_be32(ihdr, (uint32_t)height);
    ihdr.push_back(8); // bit depth
    ihdr.push_back(2); // color type: truecolor RGB
    ihdr.push_back(0); // compression method
    ihdr.push_back(0); // filter method
    ihdr.push_back(0); // interlace method

    std::vector<unsigned char> png;
    const unsigned char sig[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    png.insert(png.end(), sig, sig + 8);
    png_detail::append_chunk(png, "IHDR", ihdr);
    png_detail::append_chunk(png, "IDAT", idat);
    png_detail::append_chunk(png, "IEND", std::vector<unsigned char>());

    std::ofstream out(path, std::ios::binary);
    if (!out)
        return false;
    out.write((const char*)png.data(), (std::streamsize)png.size());
    return out.good();
}

#endif
