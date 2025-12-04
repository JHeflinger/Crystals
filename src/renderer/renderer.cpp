#include "renderer.h"
#include "util/parser.h"
#include "util/log.h"
#include "scene/material.h"
#include "scene/cie.h"
#include "renderer/config.h"
#include <thread>
#include <cstring>

#define PROGRESS_REPORT true
#define THREAD_HANDFUL 100

Renderer::Renderer() {
    MaterialUtils::initGlobalMaterials();
	CIE::init();
}

Image Renderer::render(std::string filepath, size_t w, size_t h) {
    Scene sd = Parser::parse(filepath);
    return render(sd, w, h);
}

Image Renderer::render(Scene scene, size_t w, size_t h) {
    Image img{};
    if (!scene.validated) {
        WARN("Unable to render invalid scene");
        return img;
    }
    size_t cores = std::thread::hardware_concurrency();
    m_counter = 0;
    img.w = w;
    img.h = h;
    img.colors.assign(w*h, glm::vec3(0));
    scene.camera.update(w, h);
    std::vector<std::thread> threads;
    long long start = TIME();
    if (PROGRESS_REPORT) INFO("Generating BVH...");
    scene.bvh = BVH::create(scene.primitives);
    if (PROGRESS_REPORT) INFO("Rendering rays...")
    img.prepare = ((float)(TIME() - start) / 1000.0f);
    size_t base = (h*w)/cores;
    size_t extra = (h*w)%cores;
    int pixels = w*h;
	m_threadpool = pixels;
	if (GlobalConfig::denoise()) m_denoiser = DenoiseUtils::generateBuffer(w, h);
    for (size_t i = 0; i < cores; i++) {
        size_t start = i * base + std::min(i, extra);
        size_t count = base + (i < extra ? 1 : 0);
        threads.emplace_back(&Renderer::renderPixels, this, start, count, std::ref(img), std::ref(scene));
    }
    while (PROGRESS_REPORT) {
        int counter = 0;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            counter = m_counter;
        }
        float pct = 100.0f*((float)counter)/((float)pixels);
        char backspace_buffer[128] = { 0 };
        char eq_buffer[64] = { 0 };
        char sp_buffer[64] = { 0 };
        char pct_buffer[12] = { 0 };
        char sp2_buffer[12] = { 0 };
        memset(eq_buffer, '=', 64);
        memset(sp_buffer, ' ', 64);
        sprintf(pct_buffer, "%.3f", pct);
        memset(sp2_buffer, ' ', 12);
        memset(backspace_buffer, '\b', 72);
        int b = pct/2;
        eq_buffer[b + 1] = '\0';
        sp_buffer[50 - b] = '\0';
        sp2_buffer[7 - strlen(pct_buffer)] = '\0';
        printf("Progress: [%s%s] %s%%%s", eq_buffer, sp_buffer, pct_buffer, sp2_buffer);
        if (counter >= pixels) {
            printf("\n");
            break;
        }
        printf("%s", backspace_buffer);
    }
    for (auto& thread : threads) thread.join();
	long long post = TIME();	
    img.post = ((float)(TIME() - post) / 1000.0f);
    start = TIME() - start;
    img.time = ((float)start) / 1000.0f;
    img.post = ((float)(TIME() - post) / 1000.0f);
    return img;
}

bool Renderer::saveComposites(std::string filepath) {
	if (GlobalConfig::denoise()) return DenoiseUtils::save(m_denoiser, filepath);
	return true;
}

void Renderer::renderPixels(size_t start, size_t count, Image& image, Scene& scene) {
	while (true) {
		int groupstart = 0;
		int groupend = 0;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_threadpool > THREAD_HANDFUL) {
				groupend = m_threadpool;
				groupstart = m_threadpool - THREAD_HANDFUL;
				m_threadpool -= THREAD_HANDFUL;
			} else if (m_threadpool > 0) {
				groupend = m_threadpool;
				groupstart = 0;
				m_threadpool = 0;
			} else {
				return;
			}
		}
		for (int i = groupstart; i < groupend; i++) {
			image.colors[i] = scene.shade(i%image.w, i/image.w).rgb();
			if (GlobalConfig::denoise()) DenoiseUtils::evaluateAtIndex(m_denoiser, scene, image, i);
			std::lock_guard<std::mutex> lock(m_mutex);
			m_counter++;
		}
	}
}
