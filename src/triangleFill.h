/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: triangleFill.h
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#pragma once
#include <array>
#include <vector>
#include <glm/glm.hpp>

struct Face
{
  std::array<int, 3> vertexIndices;
  std::array<int, 3> normalIndices;
  std::array<int, 3> texIndices;
};

bool triangleFill(
  const std::string& path,
  std::vector<glm::vec3> &out_vertices,
  std::vector<glm::vec3> &out_normals,
  std::vector<glm::vec3> &out_texcoords,
  std::vector<Face>& out_faces
);

void backgroundStarsConfig(float ox, float oy);