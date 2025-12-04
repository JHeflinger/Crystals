#include "image.h"
#include "util/log.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

bool Image::save(std::string filepath) {
    if (colors.size() != w*h) {
        ERROR("Unable to save image with wrong size data");
        return false;
    }
    std::vector<unsigned char> data;
    data.resize(w*h*3);
    for (int i = 0; i < w*h; i++) {
        glm::vec3 c = glm::clamp(colors[i], glm::vec3(0.0f), glm::vec3(1.0f));
        data[i*3+0] = static_cast<unsigned char>(c.r * 255.0f);
        data[i*3+1] = static_cast<unsigned char>(c.g * 255.0f);
        data[i*3+2] = static_cast<unsigned char>(c.b * 255.0f);
    }
    return stbi_write_png(filepath.c_str(), w, h, 3, data.data(), w*3) != 0;
}
