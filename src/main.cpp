#include "util/log.h"
#include "util/parser.h"
#include "renderer/renderer.h"
#include "renderer/config.h"
#include "scene/scene.h"
#include "util/parser.h"
#include "util/jlm.h"

#define VIDEO true
#define VSTART 0

void rotatescene(Scene& scene, float r) {
	for (int i = 0; i < scene.primitives.size(); i++) {
		if (scene.primitives[i].type == TRIANGLE) {
			scene.primitives[i].v1 = jlm::rotate(scene.primitives[i].v1, r, glm::vec3(0, 1.0f, 0.0f));
			scene.primitives[i].v2 = jlm::rotate(scene.primitives[i].v2, r, glm::vec3(0, 1.0f, 0.0f));
			scene.primitives[i].v3 = jlm::rotate(scene.primitives[i].v3, r, glm::vec3(0, 1.0f, 0.0f));
		} else {
			scene.primitives[i].v1 = jlm::rotate(scene.primitives[i].v1, r, glm::vec3(0, 1.0f, 0.0f));
        }
	}
}

void rotatecamera(Scene& scene, float t) {
    //float theta = 2.0f * M_PI * (t/300.0f);
    //double x = 1.0f * std::cos(theta);
    //double y = 1.0f * std::sin(theta);
    //scene.camera.look = glm::normalize(glm::vec3(x, 0.0f, y));
    scene.camera.position += glm::vec3(0.05f, 0, 0);
    scene.camera.update(scene.camera.width, scene.camera.height);
}

int main (int argc, char *argv[]) {
	if (argc != 6) {
        FATAL("Wrong number of input arguments detected - correct format:\n\t  program.exe <input_path> <output_path> <samples> <width> <height>");
	}
	GlobalConfig::pathSamples(std::stoi(std::string(argv[3])));
	INFO("Overriding # of path samples to %d", GlobalConfig::pathSamples());
    Renderer renderer;
    INFO("Parsing scene...");
	Scene scene = Parser::parse(std::string(argv[1]));
    INFO("Rendering scene...");
    if (!VIDEO) {
        Image image = renderer.render(scene, std::stoi(std::string(argv[4])), std::stoi(std::string(argv[5])));
        INFO("Finished rendering in %.3f seconds!", image.time);
        INFO("Time breakdown:\n\t  Preprocessing: %.3f seconds\n\t  Rendering: %.3f seconds\n\t  PostProcessing: %.3f seconds", image.prepare, image.time - image.prepare - image.post, image.post);
        INFO("Saving image...");
        if (image.save(std::string(argv[2]))) {
            INFO("Finished saving image!");
        } else {
            ERROR("Unable to save image");
        }
    } else {
        for (int i = 0; i < 300; i++) {
            if (i >= VSTART) {
		        Image image = renderer.render(scene, std::stoi(std::string(argv[4])), std::stoi(std::string(argv[5])));
		        INFO("Finished rendering image %d in %.3f seconds!", i, image.time);
		        image.save("videos/raws/i_" + std::to_string(i) + ".png");
            }
		    //rotatescene(scene, glm::radians(1.2f));
            rotatecamera(scene, (float)i);
	    }
    }
    return 0;
}
