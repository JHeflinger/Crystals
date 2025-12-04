#pragma once

#include <stddef.h>
#include <glm/glm.hpp>
#include <vector>

#define NMSTART 100.0f
#define NMEND 700.0f
#define NMSAMPLES 10
#define NMSAMPLESIZE ((NMEND - NMSTART)/((float)NMSAMPLES))

class Fourier;

class Spectrum {
public:
    Spectrum(float v = 0.0f);
    Spectrum(std::vector<float> lambdas, std::vector<float> values);
    Spectrum(Fourier f);
public:
    int bin(float wavelength);
    void translate(Fourier f);
    void set(float value);
    bool black();
    float average(float start, float end);
	float max();
    glm::vec3 xyz();
    glm::vec3 rgb();
public:
    Spectrum& operator+=(const Spectrum& s);
    Spectrum operator+(const Spectrum& s) const;
    Spectrum& operator-=(const Spectrum& s);
    Spectrum operator-(const Spectrum& s) const;
    Spectrum& operator*=(const Spectrum& s);
    Spectrum operator*(const Spectrum& s) const;
    Spectrum& operator/=(const Spectrum& s);
    Spectrum operator/(const Spectrum& s) const;

    Spectrum& operator+=(const Fourier& s);
    Spectrum operator+(const Fourier& s) const;
    Spectrum& operator-=(const Fourier& s);
    Spectrum operator-(const Fourier& s) const;
    Spectrum& operator*=(const Fourier& s);
    Spectrum operator*(const Fourier& s) const;
    Spectrum& operator/=(const Fourier& s);
    Spectrum operator/(const Fourier& s) const;

    Spectrum operator*(float f) const;
    Spectrum operator/(float f) const;
    float& operator[](int i);
public:
	static Spectrum isolate(const Spectrum& s, int wavelength);
    static Spectrum sqrt(const Spectrum& s);
    static Spectrum lerp(float t, const Spectrum& s1, const Spectrum& s2);
    static Spectrum clamp(const Spectrum& s, float low, float high);
    static float wavelength(int i);
public:
    bool nan() const;
    inline std::vector<float> samplesCopy() {
        std::vector<float> samples;
        for (int i = 0; i < NMSAMPLES; i++) samples.push_back(m_samples[i]);
        return samples;
    }
private:
    float m_samples[NMSAMPLES];
};

Spectrum operator*(float t, const Spectrum& s);
