#include "material.h"
#include "util/jlm.h"
#include "util/log.h"
#include "util/noise.h"
#include "util/optics.h"
#include "scene/scene.h"
#include <iostream>
#include <ostream>

Material g_default_material;
Material g_air_material;
Material g_fog_material;
bool g_materials_initialized = false;

Material::Material() {
    configureDefault(); 
}

// Phase function
float HenyeyGreenstein(float cosTheta, float g) {
    float denom = 1.f + g*g - 2.f * g * cosTheta;
    return (1.f - g*g) / (4.f * float(M_PI) * pow(denom, 1.5f));
}

glm::vec3 sampleHG(const glm::vec3& wo, float g, float u1, float u2, float* outPdf) {
    float cosTheta;
    if (fabsf(g) < 1e-3f) {
        cosTheta = 1.0f - 2.0f * u1;
    } else {
        const float gg = g * g;
        const float t  = (1.0f - gg) / (1.0f - g + 2.0f * g * u1);
        cosTheta = (1.0f + gg - t * t) / (2.0f * g);
        cosTheta = glm::clamp(cosTheta, -1.0f, 1.0f);
    }
    const float sinTheta = sqrtf(fmaxf(0.0f, 1.0f - cosTheta * cosTheta));
    const float phi = 2.0f * float(M_PI) * u2;
    const glm::vec3 w = -glm::normalize(wo);
    const glm::vec3 a = (fabsf(w.x) > 0.9f) ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
    const glm::vec3 u = glm::normalize(glm::cross(a, w));
    const glm::vec3 v = glm::cross(w, u);
    glm::vec3 wi = glm::normalize(sinTheta * cosf(phi) * u
                                  + sinTheta * sinf(phi) * v
                                  + cosTheta * w);

    if (outPdf) {
        const float cosTh = glm::dot(-wo, wi);        // angle we just sampled
        *outPdf = HenyeyGreenstein(cosTh, g);
    }
    return wi;
}
std::vector<Sample> Material::sample(const Hit& hit, const Medium& medium) const {
	if (m_type == LAMBERTIAN) {
		glm::vec3 wi = glm::normalize(SampleUtils::onb(hit.n, SampleUtils::hemisphereSample()));
		float pdf = std::max(0.0f, glm::dot(hit.n, wi)) / M_PI;
		return {(Sample){ wi, pdf, Spectrum(m_absorb) / M_PI, false, medium.wavelength, medium.ior}};
	} else if (m_type == DIELECTRIC) {
		float T = 1.0f;
		float distance = (hit.p - medium.previous).length();
		if (!m_diffract) {
			Sample s{};
			s.delta = true;
			s.wavelength = medium.wavelength;
			if (medium.wavelength >= NMSAMPLES) s.wavelength = jlm::random01()*float(NMSAMPLES);
			if (medium.material == this) T = std::exp(distance * -m_transmission.evaluate(Spectrum::wavelength(s.wavelength)));
			float ior = m_ior.evaluate(Spectrum::wavelength(s.wavelength));
			float R = Optics::DielectricFresnel(hit.d2c, hit.n, medium.ior, ior);
			if (jlm::random01() > R) { // REFRACT
				s.pdf = 1.0f - R;
				s.ior = ior;
				s.color = Spectrum(m_absorb) * s.pdf * T;
				s.incoming = glm::normalize(glm::refract(hit.d2c, hit.n, medium.ior / ior));
			} else { // REFLECT
				s.pdf = R;
				s.wavelength = medium.wavelength;
				s.ior = medium.ior;
				s.color = (medium.material == this ? Spectrum(1.0f) : Spectrum(m_absorb)) * s.pdf * T;
				s.incoming = -hit.d2r;
			}
			return {s};
		} else {
			std::vector<Sample> samples;
			for (int i = 0; i < NMSAMPLES; i++) {
				Sample s{};
				s.delta = true;
				if (medium.wavelength < NMSAMPLES) i = medium.wavelength;
				s.wavelength = i;
				float ior = m_ior.evaluate(Spectrum::wavelength(s.wavelength));
				float R = Optics::DielectricFresnel(hit.d2c, hit.n, medium.ior, ior);
				Spectrum absorbtion = Spectrum::isolate(Spectrum(m_absorb), s.wavelength);
				if (medium.material == this) T = std::exp(distance * -m_transmission.evaluate(Spectrum::wavelength(s.wavelength)));
				if (jlm::random01() > R) { // REFRACT
					s.pdf = 1.0f - R;
					s.ior = ior;
					s.incoming = glm::normalize(glm::refract(hit.d2c, hit.n, medium.ior / ior));
				} else { // REFLECT
					if (medium.material == this) absorbtion = Spectrum(1.0f);
					s.pdf = R;
					s.ior = medium.ior;
					s.incoming = -hit.d2r;
				}
				s.color = absorbtion * s.pdf * T;
				samples.push_back(s);
				if (medium.wavelength < NMSAMPLES) break;
			}
			return samples;
		}
    } else if (m_type == VOLUMETRIC) {
            glm::vec3 dir = (hit.p - medium.previous);
            float distance = glm::length(dir);
            glm::vec3 rayDir = glm::normalize(dir);

            const float densityScale = 1.25f;
            const int STEPS = 32;
            float weight = 0.0f;

            // basic raymarch (attenuation only)
            for (int i = 0; i < STEPS; i++) {
                glm::vec3 pos = medium.previous + rayDir * (distance * (i + 0.5f) / STEPS);
                float density = Noise::get().applyPerlin(pos);
                density = (density < 0.22f) ? 0.0f : (density - 0.5f) * 2.0f;
                weight += density * densityScale * (distance / STEPS);
            }

            float T = expf(-weight);
            Sample s{};
            s.delta = true;
            s.wavelength = medium.wavelength;
            if (medium.wavelength >= NMSAMPLES) s.wavelength = jlm::random01() * float(NMSAMPLES);
            s.ior = medium.ior;
            s.incoming = rayDir;
            s.pdf = 1.0f;
            s.transmission = T;

            Spectrum fogColor = Spectrum(1.0f);
            s.color = fogColor * (1.0f - T) + Spectrum(T);

            return {s};
    }

	FATAL("Unhandled material type detected");
	return {Sample{}};
}

void Material::configureDefault() {
    m_convert = Fourier();
    m_diffuse = Fourier();
    m_specular = Fourier();
	m_ambient = Fourier(Spectrum(0.0f));
	m_absorb = Fourier(Spectrum(0.8f));
	m_emission = Fourier(Spectrum(0.0f));
	m_shiny = 25.0f;
	m_ior = Fourier(Spectrum(1.0f));
	m_transmission = Fourier(Spectrum(0.0f));
	m_emissive = false;
	m_type = LAMBERTIAN;
	m_diffract = false;
}

void Material::configureFog() {
    m_convert = Fourier();
    m_diffuse = Fourier();
    m_specular = Fourier();
    m_ambient = Fourier(Spectrum(1.f));
    m_absorb = Fourier(Spectrum(0.7f));
    m_emission = Fourier(Spectrum(0.f));
    m_shiny = 25.0f;
    m_ior = Fourier(Spectrum(1.0f));
    m_transmission = Fourier(Spectrum(0.0f));
    m_emissive = false;
    m_type = VOLUMETRIC;
    m_diffract = false;
}

void Material::configureAir() {
    m_convert = Fourier();
    m_diffuse = Fourier();
    m_specular = Fourier();
    m_ambient = Fourier();
    m_absorb = Fourier();
	m_emission = Fourier();
    m_shiny = 0.0f;
	m_type = LAMBERTIAN;
    m_ior = Fourier(Spectrum(1.0f));
	m_transmission = Fourier(Spectrum(0.0f));
	m_emissive = false;
	m_diffract = false;
}

void MaterialUtils::initGlobalMaterials() {
    g_default_material.configureDefault();
    g_air_material.configureAir();
    g_fog_material.configureFog();
}

Material* MaterialUtils::DefaultMaterial() {
    return &g_default_material;
}

Material* MaterialUtils::AirMaterial() {
    return &g_air_material;
}

Material* MaterialUtils::FogMaterial() {
    return &g_fog_material;
}


glm::vec3 SampleUtils::onb(const glm::vec3& normal, const glm::vec3& local) {
	glm::vec3 w = normal;
	glm::vec3 a = (fabs(w.x) > 0.9f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
	glm::vec3 v = glm::normalize(glm::cross(w, a));
	glm::vec3 u = glm::normalize(glm::cross(w, v));
	return local.x*u + local.y*v + local.z*w;
}

glm::vec3 SampleUtils::hemisphereSample() {
	float r1 = jlm::random01();
	float r2 = jlm::random01();
	float phi = 2.0f*M_PI*r1;
	double x = std::cos(phi)*std::sqrt(1 - r2);
	double y = std::sin(phi)*std::sqrt(1 - r2);
	double z = std::sqrt(r2);
	return glm::vec3(x, y, z);
}
