#include "parser.h"
#include "util/log.h"
#include "util/jlm.h"
#include <fstream>
#include <sstream>
#include <filesystem>

std::vector<std::string> lineargs(std::string line) {
    std::vector<std::string> result;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

bool parseFloat(std::string str, float& value) {
    char* end = nullptr;
    value = std::strtof(str.c_str(), &end);
    if (end == str.c_str()) return false;
    while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end))) ++end;
    if (*end != '\0') return false;
    return true;
}

bool parseInt(std::string str, int& value) {
    char* end = nullptr;
    long result = std::strtol(str.c_str(), &end, 10);
    if (end == str.c_str()) return false;
    while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end))) ++end;
    if (*end != '\0') return false;
    if (result < INT_MIN || result > INT_MAX) return false;
    value = static_cast<int>(result);
    return true;
}

Fourier parseFourier(std::vector<std::string> args, int start, int end) {
	if (args.size() < 1) return Fourier();
	std::vector<float> values;
	for (int i = 0; i < args.size(); i++) {
		float f;
		if (!parseFloat(args[i], f)) {
            WARN("Invalid float detected: \"%s\"", args[i].c_str());
			continue;
		}
		values.push_back(f);
	}
	if (values.size() == 1) values.push_back(values[0]);
	return Fourier(values, start, end);
}

bool parseVertex(std::vector<std::string> args, Scene& scene) {
    if (args.size() != 4) return false;
    float v1, v2, v3;
    bool success = parseFloat(args[1], v1) && parseFloat(args[2], v2) && parseFloat(args[3], v3);
    if (!success) return false;
    scene.vertices.push_back(vertex(v1, v2, v3));
    return true;
}

bool parseNonGeometric(std::vector<std::string> args, Scene& scene) {
    if (args.size() != 4) return false;
    float v1, v2, v3;
    bool success = parseFloat(args[1], v1) && parseFloat(args[2], v2) && parseFloat(args[3], v3);
    if (!success) return false;
    scene.nongeos.push_back(nongeo(v1, v2, v3));
    return true;
}

bool parseAreaLight(std::vector<std::string> args, Scene& scene) {
    if (args.size() < 10) return false;
    int i1;
    float f1, f2;
    bool success = parseInt(args[1], i1) && parseFloat(args[2], f1) && parseFloat(args[3], f2);
    if (!success) return false;
    if (i1 == 0 || i1 > scene.vertices.size()) {
        WARN("Detected reference does not exist");
        return false;
    }
	Fourier f;
    float wx, wy, wz, hx, hy, hz;
    success = parseFloat(args[4], wx) && parseFloat(args[5], wy) && parseFloat(args[6], wz) &&
    parseFloat(args[7], hx) && parseFloat(args[8], hy) && parseFloat(args[9], hz);
    if (!success) return false;
	if (f1 != f2 && args.size() > 10) f = parseFourier(std::vector<std::string>(args.begin() + 10, args.end()), f1, f2);
    

    Light light = {
        scene.vertices[i1 - 1],
        f,
        glm::vec3(0),
        glm::vec3(0),
        0, 0,
        glm::vec3(wx, wy, wz),
        glm::vec3(hx, hy, hz)
    };
    scene.lights.push_back(light);
    return true;
}

bool parseAreaLightSphere(std::vector<std::string> args, Scene& scene, int currmat) {
    if (args.size() < 5) return false;
    int i1;
    float f1, f2;
    bool success = parseInt(args[1], i1) && parseFloat(args[2], f1) && parseFloat(args[3], f2);
    if (!success) return false;
    if (i1 == 0 || i1 > scene.vertices.size()) {
        WARN("Detected reference does not exist");
        return false;
    }
	Fourier f;
    float r;
    success = parseFloat(args[4], r);
    if (!success) return false;
	if (f1 != f2 && args.size() > 5) f = parseFourier(std::vector<std::string>(args.begin() + 5, args.end()), f1, f2);
    Light light = {
        scene.vertices[i1 - 1],
        f,
        glm::vec3(0),
        glm::vec3(0),
        0, 0,
        glm::vec3(0),
        glm::vec3(0),
        r
    };
    scene.lights.push_back(light);
    scene.lPrimitive.push_back(PrimitiveUtils::sphere(scene.vertices[i1 - 1], r + 0.001, currmat));
    return true;
}

bool parseDirectionalLight(std::vector<std::string> args, Scene& scene) {
    if (args.size() < 4) return false;
    int i1;
	float f1, f2;
    bool success = parseInt(args[1], i1) && parseFloat(args[2], f1) && parseFloat(args[3], f2);
    if (!success) return false;
    if (i1 == 0 || i1 > scene.nongeos.size()) {
        WARN("Detected reference does not exist");
        return false;
    }
	Fourier f;
	if (f1 != f2 && args.size() > 4) f = parseFourier(std::vector<std::string>(args.begin() + 4, args.end()), f1, f2);
    Light light = {
        glm::vec3(0),
        f,
        glm::vec3(0),
        glm::normalize(scene.nongeos[i1 - 1] * -1.0f),
        0, 0,
        glm::vec4(0.0f),
        glm::vec4(0.0f)
    };
    scene.lights.push_back(light);
    return true;
}

bool parseCamera(std::vector<std::string> args, Scene& scene) {
    if (args.size() != 5) return false;
    int i1, i2, i3;
    float f1;
    bool success = parseInt(args[1], i1) && parseInt(args[2], i2) &&
                   parseInt(args[3], i3) && parseFloat(args[4], f1);
    if (!success) return false;
    if (i1 == 0 || i2 == 2 || i3 == 0 || i1 > scene.vertices.size() || i2 > scene.nongeos.size() || i3 > scene.nongeos.size()) {
        WARN("Detected reference does not exist");
        return false;
    }
    scene.camera = {
        scene.vertices[i1 - 1],
        glm::normalize(scene.nongeos[i2 - 1] - scene.vertices[i1 - 1]),
        glm::normalize(scene.nongeos[i3 - 1]),
        glm::mat4(1.0f),
        f1,
        0.0f,
        0, 0
    };
    return true;
}

bool parseSphere(std::vector<std::string> args, Scene& scene, int currmat) {
    if (args.size() != 3) return false;
    int i1;
    float f1;
    bool success = parseInt(args[1], i1) && parseFloat(args[2], f1);
    if (!success) return false;
    if (i1 == 0 || i1 > scene.vertices.size()) {
        WARN("Detected reference does not exist");
        return false;
    }
    scene.primitives.push_back(PrimitiveUtils::sphere(scene.vertices[i1 - 1], f1, currmat));
    return true;
}

bool parseFace(std::vector<std::string> args, Scene& scene, int currmat) {
    if (args.size() != 4 && args.size() != 5) return false;
    int i1, i2, i3, i4;
    bool success = parseInt(args[1], i1) && parseInt(args[2], i2) &&
                   parseInt(args[3], i3) && (args.size() == 5 ? parseInt(args[4], i4) : true);
	if (!success) return false;
    scene.primitives.push_back(PrimitiveUtils::triangle(
        scene.vertices[i1 - 1],
        scene.vertices[i2 - 1],
        scene.vertices[i3 - 1],
		currmat
    ));
    if (args.size() == 5) {
        scene.primitives.push_back(PrimitiveUtils::triangle(
            scene.vertices[i1 - 1],
            scene.vertices[i3 - 1],
            scene.vertices[i4 - 1],
			currmat
        ));
    }
    return true;
}

bool parseNewmtl(std::vector<std::string> args, Scene& scene, std::string& curr) {
	if (args.size() != 2) return false;
	if (scene.matmap.find(args[1]) != scene.matmap.end()) {
		WARN("Material name \"%s\" already exists", args[1].c_str());
		return false;
	}
	scene.matmap[args[1]] = scene.materials.size();
	scene.materials.push_back(Material());
	curr = args[1];
	return true;
}

bool parseConvert(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	scene.materials[scene.matmap[curr]].configureConvert(f);
	return true;
}

bool parseDiffuse(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	scene.materials[scene.matmap[curr]].configureDiffuse(f);
	return true;
}

bool parseSpecular(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	scene.materials[scene.matmap[curr]].configureSpecular(f);
	return true;
}

bool parseAmbient(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	scene.materials[scene.matmap[curr]].configureAmbient(f);
	return true;
}

bool parseAbsorb(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	scene.materials[scene.matmap[curr]].configureAbsorb(f);
	return true;
}

bool parseIOR(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	scene.materials[scene.matmap[curr]].configureIOR(f);
	return true;
}

bool parseEmission(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	scene.materials[scene.matmap[curr]].configureEmission(f);
	return true;
}

bool parseTransmission(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() < 3) return false;
	int r1, r2;
	bool success = parseInt(args[1], r1) && parseInt(args[2], r2);
	if (!success) return false;
	Fourier f;
	if (r1 != r2 && args.size() > 3) f = parseFourier(std::vector<std::string>(args.begin() + 3, args.end()), r1, r2);
	Spectrum s;
	for (int i = 0; i < NMSAMPLES; i++) s[i] = -std::log(std::max(f.evaluate(Spectrum::wavelength(i)), 0.00001f));
	scene.materials[scene.matmap[curr]].configureTransmission(Fourier(s));
	return true;
}

bool parseShiny(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() != 2) return false;
	float f;
	bool success = parseFloat(args[1], f);
	if (!success) return false;
	scene.materials[scene.matmap[curr]].configureShiny(f);
	return true;
}

bool parseMaterialType(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() != 2) return false;
	if (args[1] == "dielectric") scene.materials[scene.matmap[curr]].configureType(DIELECTRIC);
	else if (args[1] == "lambertian") scene.materials[scene.matmap[curr]].configureType(LAMBERTIAN);
    else if (args[1] == "volumetric") scene.materials[scene.matmap[curr]].configureType(VOLUMETRIC);
	else return false;
	return true;
}

bool parseDiffract(std::vector<std::string> args, Scene& scene, const std::string& curr) {
	if (args.size() != 2) return false;
	if (args[1] == "true") scene.materials[scene.matmap[curr]].configureDiffract(true);
	else if (args[1] == "false") scene.materials[scene.matmap[curr]].configureDiffract(false);
	else return false;
	return true;
}

bool parseMaterials(std::vector<std::string> args, Scene& scene) {
	if (args.size() != 2) return false;
	std::filesystem::path p(scene.filepath);
	std::string mtlpath = (p.parent_path() / std::filesystem::path(args[1])).string();
	std::ifstream file = std::ifstream(mtlpath);
	if (!file.is_open()) {
        WARN("Unable to open file \"%s\"", mtlpath.c_str());
		return false;
	}
	std::string line;
	int linecount = 0;
	std::string curr = "";
	while (std::getline(file, line)) {
		linecount++;
		std::vector<std::string> args = lineargs(line);
		if (args.size() > 0 && args[0] != "#") {
			bool success = true;
			if (args[0] == "newmtl") {
				success = parseNewmtl(args, scene, curr);
			} else if (args[0] == "convert") {
				success = parseConvert(args, scene, curr);
			} else if (args[0] == "diffuse") {
				success = parseDiffuse(args, scene, curr);
			} else if (args[0] == "specular") {
				success = parseSpecular(args, scene, curr);
			} else if (args[0] == "ambient") {
				success = parseAmbient(args, scene, curr);
			} else if (args[0] == "absorb") {
				success = parseAbsorb(args, scene, curr);
			} else if (args[0] == "ior") {
				success = parseIOR(args, scene, curr);
			} else if (args[0] == "shiny") {
				success = parseShiny(args, scene, curr);
			} else if (args[0] == "type") {
				success = parseMaterialType(args, scene, curr);
			} else if (args[0] == "emission") {
				success = parseEmission(args, scene, curr);
			} else if (args[0] == "diffract") {
				success = parseDiffract(args, scene, curr);
			} else if (args[0] == "transmission") {
				success = parseTransmission(args, scene, curr);
			} else {
				WARN("Skipping property \"%s\", no specification implemented", args[0].c_str());
			}
			if (!success) {
				WARN("Unable to parse line %d of \"%s\"", linecount, mtlpath.c_str());
				return false;
			}
		}
	}
	return true;
}

bool setMaterial(std::vector<std::string> args, Scene& scene, int& currmat) {
	if (args.size() != 2) return false;
	if (scene.matmap.find(args[1]) == scene.matmap.end()) return false;
	currmat = scene.matmap[args[1]];
	return true;
}

Scene Parser::parse(std::string filepath) {
    Scene sd{};
    sd.gen = std::mt19937(std::random_device{}());
    sd.dis = std::uniform_real_distribution<>(0.0, 1.0);
    std::ifstream file = std::ifstream(filepath);
    if (!file.is_open()) {
        WARN("Unable to open file \"%s\"", filepath.c_str());
		return sd;
	}
	sd.filepath = filepath;
    std::string line;
    int linecount = 0;
	int currmat = -1;
    while (std::getline(file, line)) {
        linecount++;
        std::vector<std::string> args = lineargs(line);
        if (args.size() > 0 && args[0] != "#") {
            bool success = true;
            if (args[0] == "v") {
                success = parseVertex(args, sd);
            } else if (args[0] == "ng") {
                success = parseNonGeometric(args, sd);
            } else if (args[0] == "ld") {
                success = parseDirectionalLight(args, sd);
            } else if (args[0] == "la") {
                success = parseAreaLight(args, sd);
            } else if (args[0] == "lsphere") {
                success = parseAreaLightSphere(args, sd, currmat);
            } else if (args[0] == "camera") {
                success = parseCamera(args, sd);
            } else if (args[0] == "sphere") {
                success = parseSphere(args, sd, currmat);
            } else if (args[0] == "f") {
                success = parseFace(args, sd, currmat);
			} else if (args[0] == "mtllib") {
				success = parseMaterials(args, sd);
			} else if (args[0] == "usemtl") {
				success = setMaterial(args, sd, currmat);
            } else {
                WARN("Skipping property \"%s\", no specification implemented", args[0].c_str());
            }
            if (!success)
                WARN("Unable to parse line %d - invalid format detected:\n  \"%s\"", linecount, line.c_str());
        }
    }
    file.close();
    sd.validated = true;
    return sd;
}
