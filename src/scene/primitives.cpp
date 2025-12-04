#include "primitives.h"
#include "util/log.h"

Hit sphereIntersect(const Ray& ray, const Primitive& prim) {
    Hit h;
    h.t = -1.0f;
    glm::vec3 l = ray.p - prim.v1;
    float r2 = prim.v2.x*prim.v2.x;
    float hb = glm::dot(ray.d, l);
    float c = glm::dot(l, l) - r2;
    float dis = hb*hb - c;
    if (dis < 0.0f) return h;
    float sq = std::sqrt(dis);
    float t1 = -hb - sq;
    float t2 = -hb + sq;
    if (t1 < 0) t1 = t2;
    if (t2 < 0) t2 = t1;
    h.t = t1 < t2 ? t1 : t2;
    h.p = ray.p + ray.d * h.t;
    h.n = glm::normalize(h.p - prim.v1);
	h.material = prim.material;
    return h;
}

Hit triangleIntersect(const Ray& ray, const Primitive& prim) {
    Hit h;
    h.t = -1.0f;
    const float EPS = 1e-8;
    glm::vec3 ab = prim.v2 - prim.v1;
    glm::vec3 ac = prim.v3 - prim.v1;
    glm::vec3 pvec = glm::cross(ray.d, ac);
    float det = glm::dot(ab, pvec);
    if (fabs(det) < EPS) return h;
    float idet = 1.0f / det;
    glm::vec3 tvec = ray.p - prim.v1;
    float u = glm::dot(tvec, pvec) * idet;
    if (u < 0.0f || u > 1.0f) return h;
    glm::vec3 qvec = glm::cross(tvec, ab);
    float v = glm::dot(ray.d, qvec) * idet;
    if (v < 0.0f || u + v > 1.0f) return h;
    h.t = glm::dot(ac, qvec) * idet;
    h.p = ray.p + (ray.d*h.t);
    h.n = glm::normalize(glm::cross(ab, ac));
    // if (glm::dot(glm::normalize(ab), glm::normalize(ac)) < 0.0f) h.n *= -1.0f; this is wrong. but makes some incredible outputs
	h.material = prim.material;
    return h;
}

Primitive PrimitiveUtils::sphere(glm::vec3 position, float radius, int material) {
    return (Primitive) {
        SPHERE,
        position,
        glm::vec3(radius, 0, 0),
        glm::vec3(0),
		material
    };
}

Primitive PrimitiveUtils::triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, int material) {
    return (Primitive) { TRIANGLE, a, b, c, material };
}

glm::vec3 PrimitiveUtils::spherePos(Primitive sphere) {
    return sphere.v1;
}

float PrimitiveUtils::sphereRadius(Primitive sphere) {
    return sphere.v2.x;
}

AABB PrimitiveUtils::generateAABB(Primitive p) {
    AABB bb{};
    switch (p.type) {
        case SPHERE:
            bb.centroid = p.v1;
            bb.min = p.v1 - glm::vec3(p.v2.x, p.v2.x, p.v2.x);
            bb.max = p.v1 + glm::vec3(p.v2.x, p.v2.x, p.v2.x);
            break;
        case TRIANGLE:
            bb.min = glm::min(glm::min(p.v1, p.v2), p.v3);
            bb.max = glm::max(glm::max(p.v1, p.v2), p.v3);
            bb.centroid = glm::vec3(
                ((bb.max.x - bb.min.x)/2.0f) + bb.min.x,
                ((bb.max.y - bb.min.y)/2.0f) + bb.min.y,
                ((bb.max.z - bb.min.z)/2.0f) + bb.min.z
            );
            break;
        default:
            FATAL("Unhandled primitive type detected");
            break;
    }
    return bb;
}

Hit PrimitiveUtils::intersect(const Ray& ray, const Primitive& p) {
    switch (p.type) {
        case SPHERE:
            return sphereIntersect(ray, p); 
            break;
        case TRIANGLE:
            return triangleIntersect(ray, p);
        default:
            FATAL("Unhandled primitive type detected");
            break;
    }
    Hit h{};
    h.t = -1.0f;
    return h;
}
