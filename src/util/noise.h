#pragma once

#include "glm/glm.hpp"

class Noise
{
public:
    Noise();
    static Noise& get() {
        static Noise instance;
        return instance;
    }
    float applyPerlin(glm::vec3 p);
    float applyWorley(glm::vec3 p);
    float applyPerlinWorley(glm::vec3 p);
};
