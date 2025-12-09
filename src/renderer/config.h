#pragma once

struct Config {
	int mindepth = 3;
	int maxdepth = 1000;
	int pathsamples = 8;
	bool pathtrace = true;
	bool denoise = true;
	int pppasses = 2;
};

namespace GlobalConfig {
	int minDepth();
	int maxDepth();
	int pathSamples();
	bool pathtrace();
	bool denoise();
	int pppasses();
	void minDepth(int i);
	void maxDepth(int i);
	void pathSamples(int i);
	void pathTrace(bool b);
	void denoise(bool b);
	void pppasses(int i);
};
