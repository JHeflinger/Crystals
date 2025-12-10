#pragma once

#include "scene/primitives.h"
#include "scene/camera.h"
#include "scene/hit.h"
#include "scene/bvh.h"
#include "scene/spectrum.h"
#include "scene/material.h"
#include "scene/fourier.h"
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <random>

typedef glm::vec3 vertex;
typedef glm::vec3 nongeo;

struct Light { 
    glm::vec3 position;
    Fourier color;
    glm::vec3 attenuation;
    glm::vec3 direction;
    float penumbra;
    float angle;
    glm::vec3 hvec;
    glm::vec3 wvec;
    float radius = 0.0f;
};

struct DirectLightData {
    Spectrum color;
    float diffuse;
    float specular;
    glm::vec3 d2l;
};

struct Medium {
    float ior;
    int bounces;
	Material* material;
	Spectrum throughput;
	int wavelength;
	glm::vec3 previous;
	float depth;
};

struct Scene {
	std::string filepath;
    bool validated;
    Camera camera;
    std::vector<vertex> vertices;
    std::vector<nongeo> nongeos;
    std::vector<Light> lights;
    std::vector<Primitive> primitives;
    std::vector<Primitive> lPrimitive;
    std::vector<NodeBVH> bvh;
    std::vector<NodeBVH> bvh2;
	std::vector<Material> materials;
	std::unordered_map<std::string, int> matmap;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;
    Spectrum shade(int x, int y);
    Spectrum shade(const Ray& ray, const Medium& medium, int recur);
	void pollMetadata(const Ray& ray, glm::vec3& n, glm::vec3& p, glm::vec3& a) const;
private:
    Hit intersect(const Ray& ray) const;
    Hit intersect2(const Ray& ray) const;
    Hit traverse(const Ray& ray, size_t ind) const;
    Hit traverse2(const Ray& ray, size_t ind) const;
    Spectrum rayColor(const Hit& hit, const Medium& medium, int recur);
	Spectrum pathColor(const Hit& hit, const Medium& medium, int recur, bool light);
    bool sampleAreaLight(const Light& light, const Hit& hit);
};

namespace SceneUtils {
    static DirectLightData directLight(const Light& light, const Hit& hit, const Material& mat);
};
