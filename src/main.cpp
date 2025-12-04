#include "util/log.h"
#include "util/parser.h"
#include "renderer/renderer.h"
#include "renderer/config.h"
#include "scene/scene.h"
#include "util/parser.h"
#include "util/jlm.h"

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
    Image image = renderer.render(scene, std::stoi(std::string(argv[4])), std::stoi(std::string(argv[5])));
    INFO("Finished rendering in %.3f seconds!", image.time);
    INFO("Time breakdown:\n\t  Preprocessing: %.3f seconds\n\t  Rendering: %.3f seconds\n\t  PostProcessing: %.3f seconds", image.prepare, image.time - image.prepare - image.post, image.post);
    INFO("Saving image...");
    if (image.save(std::string(argv[2]))) {
        INFO("Finished saving image!");
    } else {
        ERROR("Unable to save image");
    }
    return 0;
}
