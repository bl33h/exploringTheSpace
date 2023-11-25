/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: triangleFill.cpp
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#include "triangleFill.h"
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>

bool triangleFill(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec3>& out_normals, std::vector<glm::vec3>& out_texcoords, std::vector<Face>& out_faces)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Failed to open the file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string lineHeader;
        iss >> lineHeader;

        if (lineHeader == "v")
        {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_vertices.push_back(vertex);
        }
        else if (lineHeader == "vn")
        {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            out_normals.push_back(normal);
        }
        else if (lineHeader == "vt")
        {
            glm::vec3 tex;
            iss >> tex.x >> tex.y >> tex.z;
            out_texcoords.push_back(tex);
        }
        else if (lineHeader == "f")
        {
            Face face;
            for (int i = 0; i < 3; ++i)
            {
                std::string faceData;
                iss >> faceData;

                std::replace(faceData.begin(), faceData.end(), '/', ' ');

                std::istringstream faceDataIss(faceData);
                faceDataIss >> face.vertexIndices[i] >> face.texIndices[i] >> face.normalIndices[i];

                face.vertexIndices[i]--;
                face.normalIndices[i]--;
                face.texIndices[i]--;
            }
            out_faces.push_back(face);
        }
    }

    return true;
}