#pragma once

#include "glm/glm.hpp"
#include <random>

#define MAT4(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) glm::mat4(a,e,i,m,b,f,j,n,c,g,k,o,d,h,l,p)
#define CLAMP(x, low, high) (std::max((low), std::min((high), x)))

// not allowed to use certain  handy dandy glm functions? no problem! jlm (jason glm) is here!

namespace jlm {
	inline float random01() {
		static std::mt19937 rng(std::random_device{}());
		static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		return dist(rng);
	}

    inline float clamp(float x, float min, float max) {
        return std::min(std::max(x, min), max);
    }

	inline glm::vec3 reflect(const glm::vec3& incident, const glm::vec3& normal) {
		return incident - 2.0f * glm::dot(normal, incident) * normal;
	}

	inline glm::vec3 mix(const glm::vec3& x, const glm::vec3& y, float a) {
		return x * (1.0f - a) + y * a;
	}

	inline glm::vec4 mix(const glm::vec4& x, const glm::vec4& y, float a) {
		return x * (1.0f - a) + y * a;
	}

    inline glm::mat4 perspective(float fovy, float aspect, float nearclip, float farclip) {
        float half = std::tan(fovy / 2.0f);
        glm::mat4 result = glm::mat4(0);
        result[0][0] = 1.0f / (aspect * half);
        result[1][1] = 1.0f / half;
        result[2][2] = -(farclip + nearclip) / (farclip - nearclip);
        result[2][3] = -1.0f;
        result[3][2] = -(2.0f * farclip * nearclip) / (farclip - nearclip);
        return result;
    }

    inline float distance(const glm::vec3& a, const glm::vec3& b) {
        glm::vec3 d = a - b;
        return std::sqrt(dot(d, d));
    }

    inline glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
        glm::vec3 f = glm::normalize(center - eye);
        glm::vec3 s = glm::normalize(glm::cross(f, up));
        glm::vec3 u = glm::cross(s, f);
        glm::mat4 result = glm::mat4(1.0f);
        result[0][0] = s.x;
        result[1][0] = s.y;
        result[2][0] = s.z;
        result[0][1] = u.x;
        result[1][1] = u.y;
        result[2][1] = u.z;
        result[0][2] = -f.x;
        result[1][2] = -f.y;
        result[2][2] = -f.z;
        result[3][0] = -glm::dot(s, eye);
        result[3][1] = -glm::dot(u, eye);
        result[3][2] =  glm::dot(f, eye);
        return result;
    }

    inline glm::mat4 scale(const glm::mat4& m, const glm::vec3& v) {
        glm::mat4 result = glm::mat4(1.0f);
        result[0][0] = v.x;
        result[1][1] = v.y;
        result[2][2] = v.z;
        return m * result;
    }

    inline glm::mat4 rotate(const glm::mat4& m, float angle, const glm::vec3& axis) {
        glm::vec3 a = glm::normalize(axis);
        float c = cos(angle);
        float s = sin(angle);
        glm::mat4 r = glm::mat4(1.0f);
        r[0][0] = c + (1.0f - c) * a.x * a.x;
        r[0][1] = (1.0f - c) * a.x * a.y + s * a.z;
        r[0][2] = (1.0f - c) * a.x * a.z - s * a.y;
        r[1][0] = (1.0f - c) * a.y * a.x - s * a.z;
        r[1][1] = c + (1.0f - c) * a.y * a.y;
        r[1][2] = (1.0f - c) * a.y * a.z + s * a.x;
        r[2][0] = (1.0f - c) * a.z * a.x + s * a.y;
        r[2][1] = (1.0f - c) * a.z * a.y - s * a.x;
        r[2][2] = c + (1.0f - c) * a.z * a.z;
        return m * r;
    }

    inline glm::mat4 translate(const glm::mat4& m, const glm::vec3& v) {
        glm::mat4 result = m;
        result[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
        return result;
    }

	inline glm::vec3 rotate(const glm::vec3& v, float angle, const glm::vec3& axis) {
		glm::mat4 rot = rotate(glm::mat4(1.0f), angle, axis);
		glm::vec4 point = glm::vec4(v, 1.0f);
		point = rot * point;
		return glm::vec3(point);
	}

}
