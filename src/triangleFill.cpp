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
#include "framebuffer.h"
#include "FastNoise.h"
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

void backgroundStarsConfig(float ox, float oy) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            FastNoiseLite noiseGenerator;
            noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
            float scale = 1000.0f;
            float noiseValue = noiseGenerator.GetNoise((x + (ox * 100.0f)) * scale, (y + oy * 100.0f) * scale);

            if (noiseValue > 0.997f) {
                float starSize = noiseValue * 3.0f;
                starSize = std::max(2.8f, starSize);
                float starIntensity = (noiseValue + 1.0f) / 2.0f;
                starIntensity = std::min(1.0f, starIntensity * 1.5f);

                Color starColor = {
                    static_cast<uint8_t>(starIntensity * 246),
                    static_cast<uint8_t>(starIntensity * 218),
                    static_cast<uint8_t>(starIntensity * 255),
                    255
                };

                FragColor starFragColor;
                starFragColor.color = starColor;
                starFragColor.z = starIntensity;

                int startX = x - static_cast<int>(starSize / 2);
                int startY = y - static_cast<int>(starSize / 4);
                int endX = x + static_cast<int>(starSize / 2);
                int endY = y + static_cast<int>(starSize / 2);

                for (int starX = startX; starX <= endX; starX++) {
                    for (int starY = startY; starY <= endY; starY++) {
                        if (starX >= 0 && starX < SCREEN_WIDTH && starY >= 0 && starY < SCREEN_HEIGHT) {
                            framebuffer[starY * SCREEN_WIDTH + starX] = starFragColor;
                        }
                    }
                }
            } else {
                framebuffer[y * SCREEN_WIDTH + x] = blank;
            }
        }
    }
}