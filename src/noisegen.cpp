#include "noisegen.h"
#include <iostream>

GLuint noisegen::generateNoiseTexture(int width, int height, int layers, int seed) {
	// Create a texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_3D, texture);

	// Generate noise
	FastNoiseSIMD * noise = FastNoiseSIMD::NewFastNoiseSIMD(seed);

	noise->SetFrequency(0.010);
	noise->SetFractalOctaves(3);
	noise->SetPerturbType(FastNoiseSIMD::PerturbType::GradientFractal);
	noise->SetPerturbAmp(0.2);
	noise->SetPerturbFractalOctaves(4);
	noise->SetCellularReturnType(FastNoiseSIMD::CellularReturnType::Distance2Cave);
	noise->SetCellularDistanceFunction(FastNoiseSIMD::CellularDistanceFunction::Natural);
	
	std::cout << "Generating noise, please wait... ";
	std::cout.flush();
	float * noise_data = noise->GetCellularSet(0, 0, 0, layers, height, width);
	std::cout << "Done" << std::endl;

	// Upload data to texture
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, width, height, layers, 0, GL_RED, GL_FLOAT, noise_data);

	// Free noise data
	FastNoiseSIMD::FreeNoiseSet(noise_data);

	// Set texture params
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Done
	return texture;
}
