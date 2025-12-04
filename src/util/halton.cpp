#include "halton.h"
#include <random>
#include <algorithm>
#include "util/log.h"

std::vector<int> permutation(int base, uint32_t seed) {
    std::vector<int> perm(base);
    for (int i = 0; i < base; i++) perm[i] = i;
    std::mt19937 rng(seed);
    std::shuffle(perm.begin(), perm.end(), rng);
    return perm;
}

double radicalInverse(int index, int base, const std::vector<int> &perm) {
    double invBase = 1.0 / base;
    double reversed = 0.0;
    double f = invBase;
    while (index > 0) {
        int digit = index % base;
        digit = perm[digit];
        reversed += digit * f;
        index /= base;
        f *= invBase;
    }
    return reversed;
}

uint32_t hashxy(uint32_t x, uint32_t y) {
    uint32_t h = x * 0x1f123bb5 ^ y * 0x5f356495;
    h ^= h >> 16;
    h *= 0x7feb352d;
    h ^= h >> 15;
    h *= 0x846ca68b;
    h ^= h >> 16;
    return h;
}

std::vector<glm::vec2> Halton::generate(int count, size_t x, size_t y) {
    uint32_t seed = hashxy(x, y);
    std::vector<glm::vec2> samples;
    samples.reserve(count);
    std::vector<int> perm2 = permutation(2, seed * 73856093u + 1);
    std::vector<int> perm3 = permutation(3, seed * 19349663u + 2);
    for (int i = 1; i <= count; i++) {
        double x = radicalInverse(i, 2, perm2);
        double y = radicalInverse(i, 3, perm3);
        samples.emplace_back((float)x, (float)y);
    }
    return samples;
}
