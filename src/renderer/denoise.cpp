#include "denoise.h"
#include "util/log.h"
#include "renderer/config.h"
#include <limits>
#include <fstream>

DenoiseBuffer DenoiseUtils::generateBuffer(size_t w, size_t h) {
	DenoiseBuffer buffer{};
    buffer.normals.assign(w*h, glm::vec3(0));
	buffer.positions.assign(w*h, glm::vec3(std::numeric_limits<float>::max()));
	buffer.albedo.assign(w*h, glm::vec3(1.0f));
	return buffer;
}

void DenoiseUtils::evaluateAtIndex(DenoiseBuffer& buffer, const Scene& scene, const Image& image, size_t i) {
	int row = i/image.w;
	int col = i%image.w;
	Ray ray = scene.camera.generateRay(image.w, image.h, col, row);
	scene.pollMetadata(
		scene.camera.generateRay(image.w, image.h, col, row),
		buffer.normals[i], buffer.positions[i], buffer.albedo[i]);
}

bool DenoiseUtils::save(const DenoiseBuffer& buffer, std::string filepath) {
	std::ofstream outFile = std::ofstream(filepath + ".normals", std::ios::binary);
    if (!outFile) {
		WARN("Unable to save composite file");
        return false;
    }
    outFile.write(reinterpret_cast<const char*>(buffer.normals.data()), buffer.normals.size() * sizeof(glm::vec3));
    outFile.close();
	outFile = std::ofstream(filepath + ".positions", std::ios::binary);
	if (!outFile) {
		WARN("Unable to save composite file");
		return false;
	}
    outFile.write(reinterpret_cast<const char*>(buffer.positions.data()), buffer.positions.size() * sizeof(glm::vec3));
    outFile.close();
	outFile = std::ofstream(filepath + ".albedo", std::ios::binary);
	if (!outFile) {
		WARN("Unable to save composite file");
		return false;
	}
    outFile.write(reinterpret_cast<const char*>(buffer.albedo.data()), buffer.albedo.size() * sizeof(glm::vec3));
    outFile.close();
	return true;
}
