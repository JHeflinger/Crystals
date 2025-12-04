#pragma once
#include "glm/glm.hpp"

namespace CIE {
	void init();
	glm::vec3 lookup(float wavelength);
};
