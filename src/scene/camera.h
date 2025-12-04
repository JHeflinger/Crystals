#pragma once

#include "scene/ray.h"

struct Camera {
    glm::vec3 position;
    glm::vec3 look;
    glm::vec3 up;
    glm::mat4 iview;
    float hangle;
    float wangle;
    size_t width;
    size_t height;
    void update(size_t w, size_t h);
    Ray generateRay(size_t x, size_t y) const;
    Ray generateRay(size_t x, size_t y, float offx, float offy) const;
};
