/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: main.cpp
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#include "print.h"
#include "noise.h"
#include "camera.h"
#include "colors.h"
#include "shaders.h"
#include "fragment.h"
#include "triangles.h"
#include "framebuffer.h"
#include "triangleFill.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <vector>
#include <cassert>

Color currentColor;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
shaderType currentshaderType = shaderType::Sun;

bool moonPresent = false;

bool init() {
    window = SDL_CreateWindow("Exploring The Universe With bl33h", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Error: Could not create SDL window." << std::endl;
        SDL_Quit();
        return false;
    }


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    setupNoise();

    return true;
}

void setColor(const Color& color) {
    currentColor = color;
}

std::vector<Vertex> vertexShaderStep(const std::vector<glm::vec3>& VBO, const Uniforms& uniforms) {
    std::vector<Vertex> transformedVertices(VBO.size() / 3);
    for (size_t i = 0; i < VBO.size() / 3; ++i) {
        Vertex vertex = { VBO[i * 3], VBO[i * 3 + 1], VBO[i * 3 + 2] };;
        transformedVertices[i] = vertexShader(vertex, uniforms);
    }
    return transformedVertices;
}

std::vector<std::vector<Vertex>> primitiveAssemblyStep(const std::vector<Vertex>& transformedVertices) {
    std::vector<std::vector<Vertex>> assembledVertices(transformedVertices.size() / 3);
    for (size_t i = 0; i < transformedVertices.size() / 3; ++i) {
        Vertex edge1 = transformedVertices[3 * i];
        Vertex edge2 = transformedVertices[3 * i + 1];
        Vertex edge3 = transformedVertices[3 * i + 2];
        assembledVertices[i] = { edge1, edge2, edge3 };
    }
    return assembledVertices;
}

std::vector<Fragment> rasterizationStep(const std::vector<std::vector<Vertex>>& assembledVertices) {
    std::vector<Fragment> concurrentFragments;
    for (size_t i = 0; i < assembledVertices.size(); ++i) {
        std::vector<Fragment> rasterizedTriangle = triangle(
            assembledVertices[i][0],
            assembledVertices[i][1],
            assembledVertices[i][2]
        );
        std::lock_guard<std::mutex> lock(std::mutex);
        for (const auto& fragment : rasterizedTriangle) {
            concurrentFragments.push_back(fragment);
        }
    }
    return concurrentFragments;
}

void fragmentShaderStep( std::vector<Fragment>& concurrentFragments, shaderType shaderType) {
for (size_t i = 0; i < concurrentFragments.size(); ++i) {
        const Fragment& fragment = fragmentShader(concurrentFragments[i], shaderType);
        point(fragment);
    }
}

void render(const std::vector<glm::vec3>& VBO, const Uniforms& uniforms) {
    std::vector<Vertex> transformedVertices = vertexShaderStep(VBO, uniforms);
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssemblyStep(transformedVertices);
    std::vector<Fragment> concurrentFragments = rasterizationStep(assembledVertices);
    fragmentShaderStep(concurrentFragments, currentshaderType);

    if (moonPresent) {
            fragmentShaderStep(concurrentFragments, currentshaderType);
    }
}

void toggleFragmentShader() {
    switch (currentshaderType) {
        case shaderType::Venus:
            currentshaderType = shaderType::Neptune;
            break;
        case shaderType::Neptune:
            currentshaderType = shaderType::Pluton;
            break;
        case shaderType::Pluton:
            currentshaderType = shaderType::Earth;
            break;
        case shaderType::Earth:
            currentshaderType = shaderType::Random;
            break;
        case shaderType::Random:
            currentshaderType = shaderType::Sun;
            break;
        case shaderType::Sun:
            currentshaderType = shaderType::Venus;
            break;
    }
}


glm::mat4 createViewportMatrix(size_t screenWidth, size_t screenHeight) {
    glm::mat4 viewport = glm::mat4(1.0f);

    viewport = glm::scale(viewport, glm::vec3(screenWidth / 2.0f, screenHeight / 2.0f, 0.5f));
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

int main(int argc, char* argv[]) {
    if (!init()) {
        return 1;
    }

    float ox = SDL_GetTicks() * 0.1f;
    float oy = SDL_GetTicks() * 0.1f;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> texCoords;
    std::vector<Face> faces;
    std::vector<glm::vec3> vertexBufferObject;

    std::string filePath = "src/objects/sphere.obj";

    if (!triangleFill(filePath, vertices, normals, texCoords,  faces)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::string spaceshipFilePath = "src/objects/sphere.obj";
    std::vector<glm::vec3> spaceshipVertices;
    std::vector<glm::vec3> spaceshipNormals;
    std::vector<glm::vec3> spaceshipTexCoords;
    std::vector<Face> spaceshipFaces;
    std::vector<glm::vec3> vertexBufferSpace;

    if (!triangleFill(spaceshipFilePath, spaceshipVertices, spaceshipNormals, spaceshipTexCoords, spaceshipFaces)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    for (const auto& face : faces)
    {
        for (int i = 0; i < 3; ++i)
        {
            glm::vec3 vertexPosition = vertices[face.vertexIndices[i]];
            glm::vec3 vertexNormal = normals[face.normalIndices[i]];
            glm::vec3 vertexTexture = texCoords[face.texIndices[i]];

            vertexBufferObject.push_back(vertexPosition);
            vertexBufferObject.push_back(vertexNormal);
            vertexBufferObject.push_back(vertexTexture);
        }
    }

    for (const auto& faceSpace : spaceshipFaces)
    {
        for (int i = 0; i < 3; ++i)
        {
            glm::vec3 vertexPositionSpace = vertices[faceSpace.vertexIndices[i]];
            glm::vec3 vertexNormalSpace = normals[faceSpace.normalIndices[i]];
            glm::vec3 vertexTextureSpace = texCoords[faceSpace.texIndices[i]];

            vertexBufferSpace.push_back(vertexPositionSpace);
            vertexBufferSpace.push_back(vertexNormalSpace);
            vertexBufferSpace.push_back(vertexTextureSpace);
        }
    }

    Uniforms uniforms;
    float a = 45.0f;
    glm::mat4 model = glm::mat4(1);
    glm::mat4 view = glm::mat4(1);
    glm::mat4 projection = glm::mat4(1);
    glm::vec3 translationVector(0.0f, 0.0f, 0.0f);
    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    glm::vec3 scaleFactor(1.0f, 1.0f, 1.0f);
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), translationVector);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleFactor);
    Camera camera(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    float fovRadians = glm::radians(45.0f);
    float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
    float nearClip = 0.1f;
    float farClip = 100.0f;
    uniforms.projection = glm::perspective(fovRadians, aspectRatio, nearClip, farClip);

    uniforms.viewport = createViewportMatrix(SCREEN_WIDTH, SCREEN_HEIGHT);
    Uint32 frameStart, frameTime;
    int speed = 8;

    bool running = true;
    while (running) {
        frameStart = SDL_GetTicks();

        if (frameTime > 0) {
            SDL_SetWindowTitle(window, "Exploring The Space With bl33h");
        }
        glm::vec3 spaceshipPosition;
        float distanceAhead = 1.0f;
        glm::mat4 SpaceModel = glm::mat4(1.0f);
        glm::mat4 rotate;
            SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    camera.rotateLeft(5.0f);
                    oy = oy + -0.5;
                    rotate = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    SpaceModel = rotate * SpaceModel;
                    break;
                case SDLK_RIGHT:
                    camera.rotateRight(5.0f);
                    oy = oy + 0.5;
                    rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    SpaceModel = rotate * SpaceModel;
                    break;
                case SDLK_UP:
                    camera.moveForward(2.0f);
                    break;
                case SDLK_DOWN:
                    camera.moveBackward(2.0f);
                    break;
                }
            }
        }

        a += 0.2;
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-a), rotationAxis);
        uniforms.model = translation * rotation * scale;

        uniforms.view = glm::lookAt(
            camera.cameraPosition, 
            camera.targetPosition, 
            camera.upVector
        );

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        clearFramebuffer();
        
        // Sun
        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::scale(sunModel, glm::vec3(3.0f));
        Uniforms sunUniforms = uniforms;
        sunUniforms.model = sunModel;
        currentshaderType = shaderType::Sun;
        render(vertexBufferObject, sunUniforms);

        // Random
        float RandomOrbitRadius = 4.0f;
        float RandomRotationAngle = glm::radians(-a);

        float RandomXPosition = RandomOrbitRadius * glm::cos(RandomRotationAngle);
        float RandomZPosition = RandomOrbitRadius * glm::sin(RandomRotationAngle);

        glm::mat4 RandomModel = glm::mat4(1.0f);
        RandomModel = glm::translate(RandomModel, glm::vec3(RandomXPosition, 0.0f, RandomZPosition));
        RandomModel = glm::rotate(RandomModel, RandomRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        RandomModel = glm::scale(RandomModel, glm::vec3(1.0f));

        Uniforms RandomUniforms = uniforms;
        RandomUniforms.model = RandomModel;
        currentshaderType = shaderType::Random;
        render(vertexBufferObject, RandomUniforms);

        glm::vec3 front = glm::normalize(camera.targetPosition - camera.cameraPosition);

        // Pluton
        float PlutonOrbitRadius = 6.0f;
        float PlutonRotationAngle = glm::radians(-a);

        float PlutonXPosition = PlutonOrbitRadius * glm::cos(PlutonRotationAngle);
        float PlutonZPosition = PlutonOrbitRadius * glm::sin(PlutonRotationAngle);

        glm::mat4 PlutonModel = glm::mat4(1.0f);
        PlutonModel = glm::translate(PlutonModel, glm::vec3(PlutonXPosition, 0.0f, PlutonZPosition));
        PlutonModel = glm::rotate(PlutonModel, PlutonRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        PlutonModel = glm::scale(PlutonModel, glm::vec3(0.8f));

        Uniforms PlutonUniforms = uniforms;
        PlutonUniforms.model = PlutonModel;
        currentshaderType = shaderType::Pluton;
        render(vertexBufferObject, PlutonUniforms);

        // Venus
        float VenusOrbitRadius = 8.0f;
        float VenusRotationAngle = glm::radians(-a);

        float VenusXPosition = VenusOrbitRadius * glm::cos(VenusRotationAngle);
        float VenusZPosition = VenusOrbitRadius * glm::sin(VenusRotationAngle);

        glm::mat4 VenusModel = glm::mat4(1.0f);
        VenusModel = glm::translate(VenusModel, glm::vec3(VenusXPosition, 0.0f, VenusZPosition));
        VenusModel = glm::rotate(VenusModel, VenusRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        VenusModel = glm::scale(VenusModel, glm::vec3(1.8f));

        Uniforms VenusUniforms = uniforms;
        VenusUniforms.model = VenusModel;
        currentshaderType = shaderType::Venus;
        render(vertexBufferObject, VenusUniforms);


        // Earth
        float earthOrbitRadius = 11.0f;
        float earthRotationAngle = glm::radians(-a);

        float earthXPosition = earthOrbitRadius * glm::cos(earthRotationAngle);
        float earthZPosition = earthOrbitRadius * glm::sin(earthRotationAngle);

        glm::mat4 earthModel = glm::mat4(1.0f);
        earthModel = glm::translate(earthModel, glm::vec3(earthXPosition, 0.0f, earthZPosition));
        earthModel = glm::rotate(earthModel, earthRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        earthModel = glm::scale(earthModel, glm::vec3(1.0f));

        Uniforms earthUniforms = uniforms;
        earthUniforms.model = earthModel;
        currentshaderType = shaderType::Earth;
        render(vertexBufferObject, earthUniforms);

        if (currentshaderType == shaderType::Earth) {
            moonPresent = true;
            float x_translation = 3.0f;
            float y_translation = 0.0f;
            float z_translation = 0.0f;
            float moonOrbitRadius = 1.0f;
            float earthRotationAngle = glm::radians(a);

            float moonRotationAngle = glm::radians(a * -1.5f);

            float moonXPosition = moonOrbitRadius * glm::cos(moonRotationAngle);
            float moonZPosition = moonOrbitRadius * glm::sin(moonRotationAngle);

            glm::mat4 moonModel = glm::mat4(1.0f);
            moonModel = glm::translate(earthModel, glm::vec3(moonXPosition, 0.0f, moonZPosition));

            glm::vec3 earthRotationAxis(0.0f, 1.0f, 0.0f);
            moonModel = glm::rotate(moonModel, earthRotationAngle, earthRotationAxis);

            float moonScaleFactor = 0.4f;
            moonModel = glm::scale(moonModel, glm::vec3(moonScaleFactor, moonScaleFactor, moonScaleFactor));

            Uniforms moonUniforms = uniforms;
            moonUniforms.model = moonModel;
            shaderType originalEarthshaderType = currentshaderType;
            currentshaderType = shaderType::Moon;
            render(vertexBufferObject, moonUniforms);
            currentshaderType = originalEarthshaderType;
        }

        // Neptune
        float NeptuneOrbitRadius = 13.0f;
        float NeptuneRotationAngle = glm::radians(-a);

        float NeptuneXPosition = NeptuneOrbitRadius * glm::cos(NeptuneRotationAngle);
        float NeptuneZPosition = NeptuneOrbitRadius * glm::sin(NeptuneRotationAngle);

        glm::mat4 NeptuneModel = glm::mat4(1.0f);
        NeptuneModel = glm::translate(NeptuneModel, glm::vec3(NeptuneXPosition, 0.0f, NeptuneZPosition));
        NeptuneModel = glm::rotate(NeptuneModel, NeptuneRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        NeptuneModel = glm::scale(NeptuneModel, glm::vec3(0.5f));

        Uniforms NeptuneUniforms = uniforms;
        NeptuneUniforms.model = NeptuneModel;
        currentshaderType = shaderType::Neptune;
        render(vertexBufferObject, NeptuneUniforms);


        renderBuffer(renderer);

        frameTime = SDL_GetTicks() - frameStart;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}