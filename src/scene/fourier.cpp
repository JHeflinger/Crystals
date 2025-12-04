#include "fourier.h"
#include "scene/spectrum.h"
#include <cmath>

Fourier::Fourier(const std::vector<float>& samples, float start, float end) : m_start(start), m_end(end) {
    const int N = samples.size();
    const float twopi = 2.0f*M_PI;
    m_a0 = 0.0f;
    m_a.clear();
    m_b.clear();
    m_a.resize(N/2);
    m_b.resize(N/2);
    for (int i = 0; i < N; i++) m_a0 += samples[i];
    m_a0 /= N;
    for (int k = 1; k <= N/2; k++) {
        float ak = 0.0f;
        float bk = 0.0f;
        for (int i = 0; i < N; i++) {
            float angle = twopi*k*(float(i)/float(N));
            float fi = samples[i];
            ak += fi * std::cos(angle);
            bk += fi * std::sin(angle);
        }
        m_a[k-1] = (2.0f/N)*ak;
        m_b[k-1] = (2.0f/N)*bk;
    }
}

Fourier::Fourier(Spectrum spectrum) 
    : Fourier(spectrum.samplesCopy(), NMSTART, NMEND) {}

float Fourier::evaluate(float t) const {
    if (m_a.size() == 0 || m_b.size() == 0) return t;
	if (t < m_start || t > m_end) return 0.0f;
    float omega = 2.0f*M_PI/float(m_end - m_start);
    float x = t - m_start;
    float sum = m_a0;
    int N = m_a.size();
    for (int k = 1; k <= N; k++)
        sum += m_a[k - 1] * std::cos(k * omega * x) + m_b[k - 1] * std::sin(k * omega * x);
    return sum;
}

Spectrum Fourier::spectrum() const {
    Spectrum s;
    for (int i = 0; i < NMSAMPLES; i++)
        s[i] = evaluate(Spectrum::wavelength(i));
    return s;
}
