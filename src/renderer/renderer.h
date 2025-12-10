#pragma once
#include "scene/scene.h"
#include "renderer/denoise.h"
#include "renderer/image.h"
#include <mutex>
#include <string>

class Renderer {
public:
    Renderer();
public:
    Image render(std::string filepath, size_t w, size_t h);
    Image render(Scene& scene, size_t w, size_t h);
	bool saveComposites(std::string filepath);
private:
    void renderPixels(size_t start, size_t count, Image& image, Scene& scene);
private:
    bool bvh = false;
    std::mutex m_mutex;
    int m_counter;
	int m_threadpool;
	DenoiseBuffer m_denoiser;
};
