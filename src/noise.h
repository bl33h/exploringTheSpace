/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: noise.h
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#pragma once
#include "./FastNoise.h"
#include <vector>

constexpr int NOISE_WIDTH = 512;
constexpr int NOISE_HEIGHT = 512;

FastNoiseLite noise;

void setupNoise() {
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}
