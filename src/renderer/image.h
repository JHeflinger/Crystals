#pragma once 

#include <string>
#include <vector>
#include "glm/glm.hpp"

struct Image {
    std::vector<glm::vec3> colors;
    size_t w;
    size_t h;
    float time;
    float prepare;
	float post;
    bool save(std::string filepath);
};
