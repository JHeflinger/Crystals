#pragma once

#include "scene/primitives.h"
#include "scene/ray.h"
#include "scene/aabb.h"
#include <vector>

enum BranchBVH {
    LEAF,
    LEFT,
    RIGHT,
    BOTH
};

struct NodeBVH {
    glm::vec3 min;
    glm::vec3 max;
    BranchBVH config;
    size_t left;
    size_t right;
};

namespace BVH {
    std::vector<NodeBVH> create(const std::vector<Primitive>& primitives);
    bool intersect(const Ray& ray, size_t ind, const std::vector<NodeBVH>& bvh);
}
