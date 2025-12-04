#include "camera.h"
#include "util/jlm.h"
#include "util/log.h"

void Camera::update(size_t w, size_t h) {
    width = w;
    height = h;
    float aspectRatio = ((float)w) / ((float)h);
    wangle = 2.0f*std::atan((aspectRatio)*std::tan(hangle/2.0f));
    glm::vec3 _w = glm::normalize(look) * -1.0f;
    glm::vec3 _v = glm::normalize(up - glm::dot(up, _w)*_w);
    glm::vec3 _u = glm::cross(_v, _w);
    glm::mat4 view = 
    MAT4(
        _u.x, _u.y, _u.z, 0,
        _v.x, _v.y, _v.z, 0,
        _w.x, _w.y, _w.z, 0,
        0, 0, 0, 1
    ) 
    * 
    MAT4(
        1, 0, 0, -position.x,
        0, 1, 0, -position.y,
        0, 0, 1, -position.z,
        0, 0, 0, 1
    );
    iview = glm::inverse(view);
}

Ray Camera::generateRay(size_t x, size_t y) const {
    return generateRay(x, y, 0.0f, 0.0f);
}

Ray Camera::generateRay(size_t x, size_t y, float offx, float offy) const {
    Ray r;
    float fy = y + offy;
    float fx = x + offx;
    float fw = width;
    float fh = height;
    r.p = position;
    r.d = glm::normalize(glm::vec3((iview * 
        glm::vec4(
            2.0f*std::tan(wangle/2.0f)*(((fx + 0.5f)/(fw)) - 0.5f),
            2.0f*std::tan(hangle/2.0f)*(((fh - 0.5f - fy)/(fh)) - 0.5f),
            -1.0f,
            1.0
        ))) - position);
    return r;
}
