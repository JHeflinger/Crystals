#include "config.h"

Config g_config;

int GlobalConfig::minDepth() {
	return g_config.mindepth;
}

int GlobalConfig::maxDepth() {
	return g_config.maxdepth;
}

int GlobalConfig::pathSamples() {
	return g_config.pathsamples;
}

bool GlobalConfig::pathtrace() {
	return g_config.pathtrace;
}

bool GlobalConfig::denoise() {
	return g_config.denoise;
}

int GlobalConfig::pppasses() {
	return g_config.pppasses;
}

void GlobalConfig::minDepth(int i) {
	g_config.mindepth = i;
}

void GlobalConfig::maxDepth(int i) {
	g_config.maxdepth = i;
}

void GlobalConfig::pathSamples(int i) {
	g_config.pathsamples = i;
}

void GlobalConfig::pathTrace(bool b) {
	g_config.pathtrace = b;
}

void GlobalConfig::denoise(bool b) {
	g_config.denoise = b;
}

void GlobalConfig::pppasses(int i) {
	g_config.pppasses = i;
}
