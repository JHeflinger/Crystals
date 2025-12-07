#include "scene.h"
#include "util/log.h"
#include "scene/spectrum.h"
#include "scene/material.h"
#include "util/optics.h"
#include "util/jlm.h"
#include "util/halton.h"
#include "renderer/config.h"
#include <iostream>

#define EPSILON 0.0001f

Spectrum Scene::shade(int x, int y) {
    int count = GlobalConfig::pathtrace() ? GlobalConfig::pathSamples() : 1;
    Spectrum s = Spectrum(0.0f);
    std::vector<glm::vec2> offsets = Halton::generate(GlobalConfig::pathSamples(), x, y);
    for (int i = 0; i < count; i++) {
        Ray ray = camera.generateRay(x, y, offsets[i].x, offsets[i].y);
        s += shade(ray, (Medium){ 1.0f, 0, MaterialUtils::AirMaterial(), Spectrum(1.0f), NMSAMPLES, ray.p}, true);
    }
    return s / float(count);
}

Spectrum Scene::shade(const Ray& ray, const Medium& medium, bool area) {
    Hit h = intersect(ray);
    if (h.t > 0.0f) return (GlobalConfig::pathtrace() ? pathColor(h, medium, area) : rayColor(h, medium));
	else if (GlobalConfig::pathtrace() && medium.material != MaterialUtils::AirMaterial()) {
		// DIRECT LIGHTING ON MISS
		Hit h2{};
		h2.n = ray.d;
		h2.p = ray.p;
		Spectrum s = Spectrum(0.0f);
		for (int i = 0; i < lights.size(); i++) {
			DirectLightData dld = SceneUtils::directLight(lights[i], h2, *medium.material);
			s += (dld.color * medium.material->diffuse().evaluate(dld.diffuse) * medium.throughput);
		}
		return s;
	}
    return Spectrum(0.0f);
}

void Scene::pollMetadata(const Ray& ray, glm::vec3& n, glm::vec3& p, glm::vec3& a) const {
	Hit h = intersect(ray);
	if (h.t > 0.0f) {
		n = h.n;
		p = h.p;
		const Material* m = h.material < 0 ? MaterialUtils::DefaultMaterial() : &(materials[h.material]);
		if (m->type() != DIELECTRIC) a = glm::clamp(Spectrum(m->absorb()).rgb(), 0.0f, 1.0f);
	}
}

Hit Scene::intersect(const Ray& ray) const {
    Hit h{};
    h.t = -1.0f;
    if (bvh.size() == 0 || !BVH::intersect(ray, 0, bvh)) return h;
    h = traverse(ray, 0);
    h.d2c = glm::normalize(ray.p - h.p);
	if (glm::dot(h.n, h.d2c) < 0.0f) h.n *= -1.0f; // comment out for more interesting outputs while raytracing diffraction
    h.d2r = glm::normalize(jlm::reflect(h.d2c, h.n));
    return h;
}

Hit Scene::traverse(const Ray& ray, size_t ind) const {
    NodeBVH node = bvh[ind];
    if (node.config == BranchBVH::LEAF) return PrimitiveUtils::intersect(ray, primitives[node.left]);
    Hit hl, hr;
    hl.t = -1.0f;
    hr.t = -1.0f;
    if (node.config == BranchBVH::LEFT || node.config == BranchBVH::BOTH)
        if (BVH::intersect(ray, node.left, bvh)) hl = traverse(ray, node.left);
    if (node.config == BranchBVH::RIGHT || node.config == BranchBVH::BOTH)
        if (BVH::intersect(ray, node.right, bvh)) hr = traverse(ray, node.right);
    Hit h;
    h.t = -1.0f;
    if (hl.t > 0 && (h.t < 0 || hl.t < h.t)) h = hl;
    if (hr.t > 0 && (h.t < 0 || hr.t < h.t)) h = hr;
    return h;
}

std::pair<float, float> getRandomPoint(std::uniform_real_distribution<> &dis, std::mt19937 &gen) {
    return {(static_cast<float>(dis(gen))), (static_cast<float>(dis(gen)))};
}

std::vector<vertex> sampleAreaLights(const Light& light, int samples, std::uniform_real_distribution<> &dis, std::mt19937 &gen) {
    glm::vec3 pos = light.position;
    std::vector<glm::vec3> positions;
    int n = std::sqrt(samples);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            auto p = getRandomPoint(dis, gen);
            float u = (i + p.first) / n;
            float v = (j + p.second) / n;
            positions.push_back(pos + u * vertex(light.wvec) + v * vertex(light.hvec));
        }
    }
    return positions;
}

Spectrum Scene::rayColor(const Hit& hit, const Medium& medium) {
    Material* m = hit.material < 0 ? MaterialUtils::DefaultMaterial() : &(materials[hit.material]);

    // DIRECT LIGHTING
    Spectrum s = m->ambient().spectrum();
    for (int i = 0; i < lights.size(); i++) {
        if (glm::length(lights[i].hvec) != 0.0f) {
            auto lightPositions = sampleAreaLights(lights[i], 100, dis, gen);
            Spectrum aggregate(0.0);
            for (const auto &p : lightPositions) {
                Light sampleLight = lights[i];
                sampleLight.position = p;
                DirectLightData dld = SceneUtils::directLight(sampleLight, hit, *m);
                auto t = intersect({ hit.p + dld.d2l * EPSILON, dld.d2l }).t;
                if (t <= 0.0f || t > hit.t) {
                    aggregate += Spectrum(m->absorb()) *
                        dld.color *
                           (m->diffuse().evaluate(dld.diffuse) +
                            m->specular().evaluate(dld.specular));
                }
            }
            s += aggregate / lightPositions.size();
        } else {
            DirectLightData dld = SceneUtils::directLight(lights[i], hit, *m);
            if (intersect((Ray){ hit.p + dld.d2l*EPSILON, dld.d2l }).t <= 0.0f) {
                s += (Spectrum(m->absorb()) * dld.color * (m->diffuse().evaluate(dld.diffuse) + m->specular().evaluate(dld.specular)));
            }
        }
        
    }

	// UPDATE THROUGHPUT
	s *= medium.throughput;
	Spectrum newT = medium.throughput * Spectrum(m->absorb());
	int newbounces = medium.bounces + 1;

    // REFLECTION/REFRACTION
    if (m->type() == DIELECTRIC && medium.bounces < GlobalConfig::maxDepth()) {
        Spectrum split = Spectrum(0.0f);
        for (int i = 0; i < NMSAMPLES; i++) {
            split[i] = Optics::DielectricFresnel(hit.d2c, hit.n, medium.ior, m->ior().evaluate(Spectrum::wavelength(i)));
        }
        glm::vec3 reflect_dir = glm::normalize(jlm::reflect(hit.d2c, hit.n));
        Spectrum reflected = shade((Ray){ hit.p + reflect_dir*EPSILON, reflect_dir }, (Medium){ medium.ior, newbounces, m, newT, medium.wavelength, hit.p }, false) * split;
        Spectrum refracted = Spectrum(0.0f);
        for (int i = 0; i < NMSAMPLES; i++) {
			if (medium.wavelength < NMSAMPLES) i = medium.wavelength;
			Material* newm = m == medium.material ? MaterialUtils::AirMaterial() : m;
            float ior = newm->ior().evaluate(Spectrum::wavelength(i));
            if (ior > 0.0f) {
                glm::vec3 refract_dir = glm::normalize(glm::refract(hit.d2c, hit.n, medium.ior / ior));
                refracted[i] += shade((Ray){ hit.p + refract_dir*EPSILON, refract_dir }, (Medium){ ior, newbounces, newm, newT, i, hit.p }, false)[i] * (1.0f - split[i]);
            }
			if (medium.wavelength < NMSAMPLES) break;
        }
        s += (refracted + reflected);
    }

    // CONVERSION
    s.translate(m->convert());

    return s*medium.throughput;
}




Spectrum Scene::pathColor(const Hit& hit, const Medium& medium, bool area) {
    Material* m = hit.material < 0 ? MaterialUtils::DefaultMaterial() : &(materials[hit.material]);



	// EMISSION
	if (m->emissive()) return medium.throughput * m->emission();

	// PATH
    Spectrum s = Spectrum(0.0f);
    for (auto light : lights) {
        if (!area) break;
        // assuming area lights
        float u = dis(gen);
        float v = dis(gen);
        auto lightPositions = sampleAreaLights(light, 25, dis, gen);
        for (const auto point : lightPositions) {
            glm::vec3 direction = point - hit.p;
            glm::vec3 dirNorm = glm::normalize(direction);
            float dist = glm::length(direction);
            glm::vec3 lightNormal = glm::normalize(glm::cross(light.wvec, light.hvec));
            float cosLight = glm::dot(-dirNorm, lightNormal);
            if (cosLight <= 0.0f) {
                continue;
            }
            float t = intersect({ hit.p + dirNorm * EPSILON, dirNorm}).t;
            if (t >= 0.0 && t < hit.t) continue;
            float area = glm::length(glm::cross(light.wvec, light.hvec));
            float cosTheta = std::max(0.0f, glm::dot(hit.n, dirNorm));
            Spectrum diffuse = Spectrum(light.color) * m->diffuse().evaluate(cosTheta) / M_PI;
            float factor = area / (dist * dist);
            s += diffuse * cosLight * factor * medium.throughput / lightPositions.size();
        }
    }
	std::vector<Sample> samples = m->sample(hit, medium);
	for (int i = 0; i < samples.size(); i++) {
		if (samples[i].pdf > 0 && medium.bounces < GlobalConfig::maxDepth()) {
			float cosTheta = std::max(0.0f, glm::dot(samples[i].incoming, hit.n));
			Spectrum newT = medium.throughput * (samples[i].color * (samples[i].delta ? 1.0f : cosTheta / samples[i].pdf));

			// RUSSIAN ROULETTE
			bool recurse = true;
			if (medium.bounces > GlobalConfig::minDepth()) {
				float p = CLAMP(newT.max(), 0.05f, 1.0f);
				if (jlm::random01() > p) recurse = false;
				else newT /= p;
			} 
			if (recurse) s += shade(
					(Ray){ hit.p + samples[i].incoming*EPSILON, samples[i].incoming },
					(Medium){ samples[i].ior, medium.bounces + 1, m, newT, samples[i].wavelength, hit.p }, false);
		}
	}

	// CONVERSION
	s.translate(m->convert());

	return s;
}

DirectLightData SceneUtils::directLight(const Light& light, const Hit& hit, const Material& mat) {
    DirectLightData dld{};
    dld.color = Spectrum(light.color);
    glm::vec3 ld = glm::vec3(0.0f);
    if (glm::length(light.direction) == 0.0f) { // point light
        dld.d2l = glm::normalize(light.position - hit.p);
        ld = dld.d2l;
    } else if (light.penumbra == 0 && light.angle == 0) { // directional light
        dld.d2l = glm::normalize(light.direction);
        ld = dld.d2l;
    } else { // spot light
        dld.d2l = glm::normalize(light.position - hit.p);
        ERROR("Not implemented yet!");
    }
    dld.diffuse = std::max(0.0f, glm::dot(hit.n, ld));
    dld.specular = (dld.diffuse > 0)*std::pow(std::max(0.0f, glm::dot(glm::normalize(jlm::reflect(ld, hit.n)), hit.d2c)), mat.shiny());
    return dld;
}
