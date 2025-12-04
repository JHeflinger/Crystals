#pragma once

#include <glm/glm.hpp>

struct Hit {
    float t;
    glm::vec3 p;
    glm::vec3 n;
    glm::vec3 d2c;
    glm::vec3 d2r;
	int material;
};
