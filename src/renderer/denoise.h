#pragma once

#include "scene/scene.h"
#include "renderer/image.h"
#include <string>

struct DenoiseBuffer {
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> albedo;
	float sigma_color  = 0.2f;   // higher → blurrier
	float sigma_normal = 256.0f; // normal similarity sensitivity
	float sigma_depth  = 0.1f;   // depends on scene scale
};

namespace DenoiseUtils {
	DenoiseBuffer generateBuffer(size_t w, size_t h);
	void evaluateAtIndex(DenoiseBuffer& buffer, const Scene& scene, const Image& image, size_t index);
	bool save(const DenoiseBuffer& buffer, std::string filepath);
};
