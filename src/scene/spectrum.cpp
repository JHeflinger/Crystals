#include "spectrum.h"
#include "scene/cie.h"
#include "scene/fourier.h"
#include "util/jlm.h"
#include "util/log.h"

Spectrum::Spectrum(float v) {
    set(v);
}

Spectrum::Spectrum(std::vector<float> lambdas, std::vector<float> values) {
    set(0.0f);
    ASSERT(lambdas.size() == values.size(), "Not enough values for the given lambdas or vice/versa");
    for (int i = 0; i < lambdas.size(); i++) {
        ASSERT(lambdas[i] >= NMSTART && lambdas[i] <= NMEND, "Lambda wavelength out of range");
        int bin = (int)((lambdas[i] - NMSTART) / NMSAMPLESIZE);
        m_samples[bin] = values[i];
    }
}

Spectrum::Spectrum(Fourier f) {
    set(0.0f);
    for (int i = 0; i < NMSAMPLES; i++)
        m_samples[i] = f.evaluate(wavelength(i));
}

int Spectrum::bin(float wavelength) {
    return CLAMP(((wavelength - NMSTART)/NMSAMPLESIZE), 0.0f, NMSAMPLES - 1.0f);
}

void Spectrum::translate(Fourier f) {
    float oldsamples[NMSAMPLES];
    for (int i = 0; i < NMSAMPLES; i++)
        oldsamples[i] = m_samples[i];
    set(0.0f);
    for (int i = 0; i < NMSAMPLES; i++)
        m_samples[bin(f.evaluate(wavelength(i)))] += oldsamples[i];
}

void Spectrum::set(float value) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] = value;
}

bool Spectrum::black() {
    for (int i = 0; i < NMSAMPLES; i++) if (m_samples[i] != 0.0f) return false;
    return true;
}

float Spectrum::average(float start, float end) {
    ASSERT(start >= NMSTART && start <= NMEND && end >= NMSTART && end <= NMEND, "Given lambdas are out of range");
    ASSERT(end > start, "Cannot calculate the average of an inverse range");
    int bstart = (int)((start - NMSTART) / NMSAMPLESIZE);
    int bend = (int)((end - NMSTART) / NMSAMPLESIZE);
    float total = 0.0f;
    for (int i = 0; i < bend + 1; i++)
        total += m_samples[i]*NMSAMPLESIZE;
    return total / (end - start);
}

float Spectrum::max() {
	float m = m_samples[0];
	for (int i = 1; i < NMSAMPLES; i++) m = std::max(m, m_samples[i]);
	return m;
}

glm::vec3 Spectrum::xyz() {
    float X = 0;
    float Y = 0;
    float Z = 0;
    for (int i = 0; i < NMSAMPLES; i++) {
        glm::vec3 c = CIE::lookup(wavelength(i));
        X += m_samples[i] * c.x * NMSAMPLESIZE;
        Y += m_samples[i] * c.y * NMSAMPLESIZE;
        Z += m_samples[i] * c.z * NMSAMPLESIZE;
    }
    return glm::vec3(X, Y, Z);
}

glm::vec3 Spectrum::rgb() {
    glm::vec3 _xyz = xyz();
    glm::vec3 _rgb = glm::vec3(
        3.240479f*_xyz[0] - 1.537150f*_xyz[1] - 0.498535f*_xyz[2],
        -0.969256f*_xyz[0] + 1.875991f*_xyz[1] + 0.041556f*_xyz[2],
        0.055648f*_xyz[0] - 0.204043f*_xyz[1] + 1.057311f*_xyz[2]
    );
    return glm::max(_rgb, glm::vec3(0.0f));
}

Spectrum& Spectrum::operator+=(const Spectrum& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] += s.m_samples[i];
    return *this;
}

Spectrum Spectrum::operator+(const Spectrum& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] += s.m_samples[i];
    return ret;
}

Spectrum& Spectrum::operator-=(const Spectrum& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] -= s.m_samples[i];
    return *this;
}

Spectrum Spectrum::operator-(const Spectrum& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] -= s.m_samples[i];
    return ret;
}

Spectrum& Spectrum::operator*=(const Spectrum& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] *= s.m_samples[i];
    return *this;
}

Spectrum Spectrum::operator*(const Spectrum& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] *= s.m_samples[i];
    return ret;
}

Spectrum& Spectrum::operator/=(const Spectrum& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] /= s.m_samples[i];
    return *this;
}

Spectrum Spectrum::operator/(const Spectrum& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] /= s.m_samples[i];
    return ret;
}

Spectrum& Spectrum::operator+=(const Fourier& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] += s.evaluate(wavelength(i));
    return *this;
}

Spectrum Spectrum::operator+(const Fourier& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] += s.evaluate(wavelength(i));
    return ret;
}

Spectrum& Spectrum::operator-=(const Fourier& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] -= s.evaluate(wavelength(i));
    return *this;
}

Spectrum Spectrum::operator-(const Fourier& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] -= s.evaluate(wavelength(i));
    return ret;
}

Spectrum& Spectrum::operator*=(const Fourier& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] *= s.evaluate(wavelength(i));
    return *this;
}

Spectrum Spectrum::operator*(const Fourier& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] *= s.evaluate(wavelength(i));
    return ret;
}

Spectrum& Spectrum::operator/=(const Fourier& s) {
    for (int i = 0; i < NMSAMPLES; i++) m_samples[i] /= s.evaluate(wavelength(i));
    return *this;
}

Spectrum Spectrum::operator/(const Fourier& s) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] /= s.evaluate(wavelength(i));
    return ret;
}

Spectrum Spectrum::operator*(float f) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] *= f;
    return ret;
}

Spectrum Spectrum::operator/(float f) const {
    Spectrum ret = *this;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] /= f;
    return ret;
}

float& Spectrum::operator[](int i) {
    return m_samples[i];
}

Spectrum Spectrum::isolate(const Spectrum& s, int wavelength) {
	Spectrum ret = Spectrum(0.0f);
	ret[wavelength] = s.m_samples[wavelength];
	return ret;
}

Spectrum Spectrum::sqrt(const Spectrum& s) {
    Spectrum ret;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] = std::sqrt(s.m_samples[i]);
    return ret;
}

Spectrum Spectrum::lerp(float t, const Spectrum& s1, const Spectrum& s2) {
    return (1.0f - t) * s1 + t * s2;
}

Spectrum Spectrum::clamp(const Spectrum& s, float low, float high) {
    Spectrum ret;
    for (int i = 0; i < NMSAMPLES; i++) ret.m_samples[i] = std::min(std::max(low, s.m_samples[i]), high);
    return ret;
}

float Spectrum::wavelength(int i) {
    ASSERT(i >= 0 && i < NMSAMPLES, "Cannot get wavelength of a bin outside the spectrum");
    return ((((float)i) + 0.5f) * NMSAMPLESIZE) + NMSTART;
}

bool Spectrum::nan() const {
    for (int i = 0; i < NMSAMPLES; i++) if (std::isnan(m_samples[i])) return true;
    return false;
}

Spectrum operator*(float t, const Spectrum& s) { return s*t; }
