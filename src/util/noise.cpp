#include "noise.h"
#include <glm/glm.hpp>

Noise::Noise() {}

inline unsigned int hash3D(int x, int y, int z) {
    unsigned int h = static_cast<unsigned int>(x) * 1619u +
                     static_cast<unsigned int>(y) * 31337u +
                     static_cast<unsigned int>(z) * 6971u;

    h ^= h >> 16;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    h *= 0xc2b2ae35u;
    h ^= h >> 16;

    return h;
}

glm::vec3 randomVec(glm::vec3 latticePoint) {
    int x = static_cast<int>(latticePoint.x);
    int y = static_cast<int>(latticePoint.y);
    int z = static_cast<int>(latticePoint.z);

    unsigned int h = hash3D(x, y, z);
    float gx = (h & 1) ? 1.0f : -1.0f;
    float gy = (h & 2) ? 1.0f : -1.0f;
    float gz = (h & 4) ? 1.0f : -1.0f;

    if (((h >> 3) & 3) == 0) gx = 0.0f;
    else if (((h >> 3) & 3) == 1) gy = 0.0f;
    else gz = 0.0f;

    return glm::normalize(glm::vec3(gx, gy, gz));
}

static inline float u01(uint32_t x) {
    return (x >> 8) * (1.0f / 16777216.0f);
}

static inline uint32_t mix32(uint32_t x) {
    x ^= x >> 16;  x *= 0x7feb352dU;
    x ^= x >> 15;  x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

static inline glm::vec3 randomPointInCell(const glm::ivec3& cell) {
    uint32_t h = hash3D(cell.x, cell.y, cell.z);
    uint32_t hx = mix32(h ^ 0x9E3779B9U);
    uint32_t hy = mix32(h ^ 0x85EBCA6BU);
    uint32_t hz = mix32(h ^ 0xC2B2AE35U);
    return glm::vec3(u01(hx), u01(hy), u01(hz));
}


float Noise::applyPerlin(glm::vec3 p) {
    float period = 7.f;
    glm::vec3 wrapped = glm::mod(p, glm::vec3(period));
    glm::vec3 i = glm::floor(wrapped);
    glm::vec3 f = wrapped - i;

    // smoothing fade
    glm::vec3 u = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);

    float result = 0.0f;
    for (int z = 0; z <= 1; z++)
        for (int y = 0; y <= 1; y++)
            for (int x = 0; x <= 1; x++) {
                glm::vec3 offset(x, y, z);
                glm::vec3 lattice = glm::mod(i + offset, glm::vec3(period));
                glm::vec3 g = randomVec(lattice);
                glm::vec3 d = f - offset;
                float w = (x ? u.x : 1.0f - u.x) *
                          (y ? u.y : 1.0f - u.y) *
                          (z ? u.z : 1.0f - u.z);
                result += w * glm::dot(g, d);
            }

    result = (result + 1.0f) * 0.5f;
    return glm::clamp(result, 0.0f, 1.0f);
}

float Noise::applyWorley(glm::vec3 p) {
    glm::vec3 cell = glm::floor(p);
    float period = 5.f;
    float minDist = 1.0f;

    for(int z = -1; z <= 1; z++)
        for(int y = -1; y <= 1; y++)
            for(int x = -1; x <= 1; x++) {
                glm::vec3 neighbor = cell + glm::vec3(x, y, z);
                glm::vec3 wrappedNeighbor = glm::mod(neighbor, glm::vec3(period));
                glm::vec3 point = neighbor + randomPointInCell(wrappedNeighbor);
                glm::vec3 diff = point - p;
                diff = glm::mod(diff + period * 0.5f, glm::vec3(period)) - period * 0.5f;
                float dist = glm::length(diff);
                minDist = abs(std::min(minDist, dist));
            }


    return glm::clamp(minDist / sqrtf(3) / 2.f, 0.f, 1.f);
}

float Noise::applyPerlinWorley(glm::vec3 p) {
    return 1.f;
}
