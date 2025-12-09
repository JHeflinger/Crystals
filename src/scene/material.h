#pragma once

#include "scene/fourier.h"
#include "scene/spectrum.h"
#include "glm/glm.hpp"
#include "scene/hit.h"

enum MaterialType {
	LAMBERTIAN,
    DIELECTRIC,
    VOLUMETRIC
};

struct Sample {
	glm::vec3 incoming;
	float pdf;
	Spectrum color;
	bool delta;
	int wavelength;
	float ior;

    // raymarching
    bool isVolume = false;
    float transmission;
    glm::vec3 eventPos = glm::vec3(0);
};

struct Medium;

class Material {
public:
    Material();
public:
	std::vector<Sample> sample(const Hit& hit, const Medium& medium) const;
public:
    void configureAir();
    void configureDefault();
    void configureFog();
public:
    const Fourier& ambient() const { return m_ambient; }
    const Fourier& convert() const { return m_convert; }
    const Fourier& absorb() const { return m_absorb; }
    const Fourier& diffuse() const { return m_diffuse; }
    const Fourier& specular() const { return m_specular; }
    const Fourier& ior() const { return m_ior; }
	const Fourier& emission() const { return m_emission; }
	const Fourier& transmission() const { return m_transmission; }
    float shiny() const { return m_shiny; }
	bool emissive() const { return m_emissive; }
    bool diffract() const { return m_diffract; }
	MaterialType type() const { return m_type; }
public:
	void configureAmbient(Fourier f) { m_ambient = f; }
	void configureConvert(Fourier f) { m_convert = f; }
	void configureAbsorb(Fourier f) { m_absorb = f; }
	void configureDiffuse(Fourier f) { m_diffuse = f; }
	void configureSpecular(Fourier f) { m_specular = f; }
	void configureIOR(Fourier f) { m_ior = f; }
	void configureEmission(Fourier f) { m_emission = f; m_emissive = !f.empty(); }
	void configureShiny(float f) { m_shiny = f; }
	void configureType(MaterialType t) { m_type = t;}
	void configureDiffract(bool b) { m_diffract = b; }
	void configureTransmission(Fourier f) { m_transmission = f; }
private:
    Fourier m_ambient;
    Fourier m_convert;
    Fourier m_absorb;
    Fourier m_diffuse;
    Fourier m_specular;
    Fourier m_ior;
	Fourier m_emission;
	Fourier m_transmission;
    float m_shiny;
	bool m_emissive;
	bool m_diffract;
	MaterialType m_type;
};

namespace MaterialUtils {
    void initGlobalMaterials();
    Material* DefaultMaterial();
    Material* AirMaterial();
    Material* FogMaterial();
};

namespace SampleUtils {
	glm::vec3 onb(const glm::vec3& normal, const glm::vec3& local);
	glm::vec3 hemisphereSample();
};
