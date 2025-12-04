#include "optics.h"
#include "util/jlm.h"
#include "util/log.h"

float Optics::DielectricFresnel(glm::vec3 d2c, glm::vec3 normal, float ior_out, float ior_in) {
	float cti = CLAMP(glm::dot(d2c, normal), -1.0f, 1.0f);
    if (cti < 0.0f) {
        cti *= -1.0f;
        std::swap(ior_out, ior_in);
    }
    float ior = ior_out / ior_in;
    float sti = std::sqrt(std::max(0.0f, 1.0f - cti*cti));
    float stt = ior*sti;
    if (stt >= 1.0f) return 1.0f;
    float ctt = std::sqrt(std::max(0.0f, 1.0f - stt*stt));
    float rs = (ior_out*cti - ior_in*ctt)/(ior_out*cti + ior_in*ctt);
    float rp = (ior_in*cti - ior_out*ctt)/(ior_in*cti + ior_out*ctt);
    return CLAMP(0.5f * (rs*rs + rp*rp), 0.0f, 1.0f);
}
