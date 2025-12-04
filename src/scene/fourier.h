#pragma once

#include <vector>

class Spectrum;

class Fourier {
public:
    Fourier() : m_a0(0), m_start(0), m_end(0) {}
	Fourier(float a0, std::vector<float> a, std::vector<float> b) : m_a0(a0), m_a(a), m_b(b), m_start(0), m_end(1) {}
    Fourier(const std::vector<float>& samples, float start, float end);
	Fourier(Spectrum spectrum);
    float evaluate(float t) const;
    Spectrum spectrum() const;
	bool empty() const { return m_start == m_end; }
private:
	float m_a0;
	std::vector<float> m_a;
	std::vector<float> m_b;
    float m_start;
    float m_end;
};
