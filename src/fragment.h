/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: fragment.h
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#pragma once
#include "colors.h"
#include <string>
#include <cstdint>
#include <iostream>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <SDL2/SDL_image.h>

extern SDL_Surface* currentTexture;
bool loadTexture(const std::string& filename);
Color getPixelFromTexture(float u, float v);
glm::vec3 getNormalFromTexture(float u, float v);

struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewport;
};

struct Fragment {
  uint16_t x;      
  uint16_t y;      
  double z;
  Color color;
  float intensity;
  glm::vec3 worldPos;
  glm::vec3 originalPos;
};

struct FragColor {
  Color color;
  double z; 
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tex;
  glm::vec3 worldPos;
  glm::vec3 originalPos;
};