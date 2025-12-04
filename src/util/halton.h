#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Halton {
    std::vector<glm::vec2> generate(int count, size_t x, size_t y);
};
