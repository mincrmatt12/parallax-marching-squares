#include "noisegen.h"
#include <iostream>

FastNoiseSIMD * noise = FastNoiseSIMD::NewFastNoiseSIMD(0);

noisegen::NoiseSet::NoiseSet(int size) {
	data = noise->GetEmptySet(size);
}

noisegen::NoiseSet::~NoiseSet() {
	FastNoiseSIMD::FreeNoiseSet(data);
}

GLuint noisegen::generateNoiseTexture(int width, int height, int layers, int seed, float * buffer) {
	// Create a texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_3D, texture);

	// Generate noise
	noise->SetSeed(seed);
	noise->SetFrequency(0.010);
	noise->SetFractalOctaves(3);
	noise->SetPerturbType(FastNoiseSIMD::PerturbType::GradientFractal);
	noise->SetPerturbAmp(0.2);
	noise->SetPerturbFractalOctaves(4);
	
	std::cout << "Generating noise, please wait... ";
	std::cout.flush();
	
	if (buffer != nullptr) {
		noise->FillSimplexFractalSet(buffer, 0, 0, 0, layers, height, width);

		// Upload data to texture
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, width, height, layers, 0, GL_RED, GL_FLOAT, buffer);
	}
	else {
		buffer = noise->GetSimplexFractalSet(0, 0, 0, layers, height, width);

		// Upload data to texture
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, width, height, layers, 0, GL_RED, GL_FLOAT, buffer);
		glFlush();

		// Free noise data
		FastNoiseSIMD::FreeNoiseSet(buffer);
	}
	std::cout << "Done" << std::endl;

	// Set texture params
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	// Done
	return texture;
}
