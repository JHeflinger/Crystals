#pragma once
#include <glm/glm.hpp>
#include "scene/aabb.h"
#include "scene/ray.h"
#include "scene/hit.h"

enum PrimitiveType {
    SPHERE,
    TRIANGLE
};

struct Primitive {
    PrimitiveType type;
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;
	int material;
};

namespace PrimitiveUtils {
    Primitive sphere(glm::vec3 position, float radius, int material);
    Primitive triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, int material);
    glm::vec3 spherePos(Primitive sphere);
    float sphereRadius(Primitive sphere);
    AABB generateAABB(Primitive p);
    Hit intersect(const Ray& ray, const Primitive& p);
};
