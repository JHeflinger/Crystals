#pragma once

#include "glm/glm.hpp"

namespace Optics {
    float DielectricFresnel(glm::vec3 d2c, glm::vec3 normal, float ior_out, float ior_in);
};
