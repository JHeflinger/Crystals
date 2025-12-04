#include "util/log.h"
#include "util/parser.h"
#include "renderer/renderer.h"
#include "renderer/config.h"
#include "scene/scene.h"
#include "util/parser.h"
#include "util/jlm.h"

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

int main (int argc, char *argv[]) {
	if (argc == 2) {
		GlobalConfig::pathSamples(std::stoi(std::string(argv[1])));
		INFO("Overriding # of path samples to %d", GlobalConfig::pathSamples());
	}
    Renderer renderer;
    INFO("Parsing scene...");
	Scene scene = Parser::parse("resources/diamond.obj");
    INFO("Rendering scene...");

    int pause = 601;

    if (pause > 600) {
        Image image = renderer.render(scene, 600, 600);
        INFO("Finished rendering in %.3f seconds!", image.time);
        INFO("Time breakdown:\n\t  Preprocessing: %.3f seconds\n\t  Rendering: %.3f seconds\n\t  PostProcessing: %.3f seconds", image.prepare, image.time - image.prepare - image.post, image.post);
        INFO("Saving image to out.png...");
        if (image.save("out.png") && renderer.saveComposites("out.png")) {
            INFO("Finished saving image!");
        } else {
            ERROR("Unable to save image");
        }
    }
	
	for (int i = 0; i < 600; i++) {
        if (i >= pause) {
		    Image image = renderer.render(scene, 600, 600);
		    INFO("Finished rendering image %d in %.3f seconds!", i, image.time);
		    image.save("videos/raws/i_" + std::to_string(i) + ".png");
        }
		rotatescene(scene, glm::radians(0.6f));
	}
    return 0;
}
