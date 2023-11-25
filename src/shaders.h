/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: shaders.h
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#pragma once
#include <glm/glm.hpp>
#include "FastNoiseLite.h"
#include "fragment.h"
#include "noise.h"
#include "print.h"

enum class shaderType {
    Earth,
    Random,
    Pluton,
    Neptune,
    Sun,
    Moon,
    Venus
};

Vertex vertexShader(const Vertex& vertex, const Uniforms& uniforms) {
    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);
    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;
    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);
    
    glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
    transformedNormal = glm::normalize(transformedNormal);

    glm::vec3 transformedWorldPosition = glm::vec3(uniforms.model * glm::vec4(vertex.position, 1.0f));

    return Vertex{
        glm::vec3(screenVertex),
        transformedNormal,
        vertex.tex,
        transformedWorldPosition,
        vertex.position
    };
}

Color interpolateColor(const Color& color1, const Color& color2, float t) {
    t = glm::clamp(t, 0.0f, 1.0f);
    return Color(
        static_cast<int>((1.0f - t) * color1.r + t * color2.r),
        static_cast<int>((1.0f - t) * color1.g + t * color2.g),
        static_cast<int>((1.0f - t) * color1.b + t * color2.b)
    );
}

float noiseGenerator(float x, float y, float z) {
    FastNoiseLite noise;
    int offsetX = 1000;
    int offsetY = 1000;
    float offsetZ = 0.6f;
    int scale = 1000;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFrequency(0.005f);

    float LandThreshold = 0.4f;

    float normalizedValue = noise.GetNoise(((x * scale) + offsetX) * offsetZ, ((y * scale) + offsetY) * offsetZ, (z * scale)* offsetZ);
    
    return (normalizedValue < LandThreshold) ? 1.0f : 0.0f;
}

float densityGenerator(float x, float y, float z) {
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.1f);

    float scale = 100.0f;
    float waterBodiesDensity = noise.GetNoise(x * scale, y * scale, z * scale);

    waterBodiesDensity = (waterBodiesDensity + 1.0f) / 2.0f;

    float waterBodiesThreshold = 0.65f;

    if (waterBodiesDensity > waterBodiesThreshold) {
        return waterBodiesDensity;
    } else {
        return 0.0f;
    }
}

Fragment fragmentShaderMoon(Fragment& fragment) {
    glm::vec3 sunCenter = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 fragmentToMoon = sunCenter - fragment.worldPos;
    float distanceToSun = glm::length(fragmentToMoon);
    float sunRadius = 1.0f;
    float glowFactor = glm::smoothstep(sunRadius, sunRadius * 1.2f, distanceToSun);

    glm::vec3 innerColor = glm::vec3(0.016, 0.090, 0.173);
    glm::vec3 middleColor = glm::vec3(0.549f, 0.612f, 0.733f);
    glm::vec3 outerColor = glm::vec3(0.180, 0.243, 0.318);

    glm::vec3 finalColor = innerColor * glowFactor + middleColor * (1.0f - glowFactor) + outerColor * (1.0f - glowFactor) * (1.0f - glowFactor);

    finalColor =  finalColor * fragment.intensity;

    glm::vec3 spotColor = glm::vec3(0.549f, 0.612f, 0.733f);
    float randomValue = glm::fract(glm::sin(fragment.worldPos.x * 5.0f) * 43758.5453f);
    finalColor = glm::mix(finalColor, spotColor, randomValue * 0.4f); 
    fragment.color = Color(finalColor.r, finalColor.g, finalColor.b);

    return fragment;
}

Fragment fragmentShaderEarth(Fragment& fragment) {
    Color SouthPole;
    Color Land;
    Color Ocean;
    Color waterBodies;
    glm::vec3 oceanColor = glm::vec3(34, 75, 156);
    glm::vec3 LandColor = glm::vec3(198, 255, 64);
    glm::vec3 waterBodiesColor = glm::vec3(148, 206, 228);

    float LandPattern = glm::smoothstep(0.25f, 1.0f, fragment.originalPos.y);
    float LandPatternSouth = glm::smoothstep(-0.35f, -1.3f, fragment.originalPos.y);

    glm::vec3 c = glm::mix(oceanColor, LandColor, LandPattern);
    glm::vec3 d = glm::mix(oceanColor, LandColor, LandPatternSouth);
    float southPoleThreshold = -0.4f;

    if (fragment.originalPos.y < southPoleThreshold) {
        Color color = Color(d.x, d.y, d.z);
        color = color * fragment.intensity;
        SouthPole = color;
    } else {
        float waterBodiesDensity = densityGenerator(fragment.originalPos.x, fragment.originalPos.y, fragment.originalPos.z);
        Color waterBodiesLayer = Color(waterBodiesColor.x / 255.0f, waterBodiesColor.y / 255.0f, waterBodiesColor.z / 255.0f);
        waterBodies = waterBodiesLayer * waterBodiesDensity;
        float noiseValue = noiseGenerator(fragment.originalPos.x, fragment.originalPos.y, fragment.originalPos.z);
        float LandThreshold = 0.5f;
        if (noiseValue < LandThreshold) {
            Land = Color(LandColor.x / 255.0f, LandColor.y / 255.0f, LandColor.z / 255.0f); // Normaliza el color a valores en el rango [0, 1]
        } else {
            Ocean = Color(oceanColor.x / 255.0f, oceanColor.y / 255.0f, oceanColor.z / 255.0f); // Normaliza el color a valores en el rango [0, 1]
        }
    }

    Color finalColor = SouthPole + Ocean + Land + waterBodies;

    fragment.color = finalColor;

    return fragment;
}

Fragment fragmentShaderNeptune(Fragment& fragment) {
    glm::vec3 hotColor = glm::vec3(0.172549f, 0.219608f, 0.541176f);
    glm::vec3 warmColor = glm::vec3(0.392157f, 0.478431f, 0.988235f);
    glm::vec3 coolColor = glm::vec3(0.0f, 1.0f, 1.0f);

    float intValue = (glm::sin(fragment.originalPos.x * 10.0f) + glm::cos(fragment.originalPos.y * 3.0f)) * 0.5f + 0.5f;
    glm::vec3 baseColor = glm::mix(glm::mix(coolColor, warmColor, intValue), hotColor, intValue);

    float noiseValue = glm::fract(glm::sin(fragment.originalPos.x * 5.0f + fragment.originalPos.y * 10.0f) * 43758.5453f);
    float whiteSpotThreshold = 0.98f;

    if (noiseValue > whiteSpotThreshold) {
        glm::vec3 whiteSpotColor = glm::vec3(1.0f, 1.0f, 1.0f);
        baseColor = glm::mix(baseColor, whiteSpotColor, 0.5f);
    }

    baseColor = baseColor * fragment.intensity;
    fragment.color = Color(baseColor.r, baseColor.g, baseColor.b);

    return fragment;
}

Fragment fragmentShaderVenus(Fragment& fragment) {
    glm::vec3 hotColor = glm::vec3(1.0, 0.18039215686, 0.4);
    glm::vec3 warmColor = glm::vec3(0.29019607843, 1.0, 0.50588235294);
    glm::vec3 coolColor = glm::vec3(0.9333, 0.5216, 0.4588);

    float heatValue = (glm::sin(fragment.originalPos.x * 7.0f) + glm::cos(fragment.originalPos.y * 7.0f)) * 0.5f + 0.5f;

    glm::vec3 baseColor = glm::mix(glm::mix(coolColor, warmColor, heatValue), hotColor, heatValue);

    float totalNoise = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < 3; ++i) {
        float noiseValue = noiseGenerator(fragment.originalPos.x * frequency, fragment.originalPos.y * frequency, fragment.originalPos.z);
        totalNoise += amplitude * noiseValue;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    float whiteSpotThreshold = -1.0f;

    if (totalNoise > whiteSpotThreshold) {
        glm::vec3 whiteSpotColor = glm::vec3(1.0f, 1.0f, 1.0f);
        baseColor = glm::mix(baseColor, whiteSpotColor, 0.5f);
    }

    float lineFrequency = 20.0f + noiseGenerator(fragment.originalPos.x, fragment.originalPos.y, fragment.originalPos.z) * 10.0f;
    float lineValue = glm::sin(fragment.originalPos.y * lineFrequency);

    if (glm::abs(lineValue) < 0.1f) {
        glm::vec3 lineColor = glm::vec3(0.9333, 0.5216, 0.4588);
        baseColor = glm::mix(baseColor, lineColor, 0.8f);
    }

    baseColor = baseColor * fragment.intensity;
    fragment.color = Color(baseColor.r, baseColor.g, baseColor.b);

    return fragment;
}


Fragment fragmentShaderSun(Fragment& fragment) {
    glm::vec3 hotColor = glm::vec3(1.0, 0.498, 0.208);
    glm::vec3 warmColor = glm::vec3(1.0f, 0.0f, 0.0f);

    float heatValue = (glm::sin(fragment.originalPos.x * 10.0f) + glm::cos(fragment.originalPos.y * 2.0f)) * 0.7f + 0.6f;

    glm::vec3 interpolatedColor = glm::mix(warmColor, hotColor, heatValue);

    interpolatedColor = interpolatedColor * fragment.intensity;
    fragment.color = Color(interpolatedColor.r, interpolatedColor.g, interpolatedColor.b);

    return fragment;
}

Fragment fragmentShaderRandom(Fragment& fragment) {
    glm::vec3 hotColor = glm::vec3(0.549, 0.286, 1.0);
    glm::vec3 warmColor = glm::vec3(0.549, 0.796, 0.047);
    glm::vec3 coolColor = glm::vec3(0.549, 0.796, 0.047);

    float heatValue = (glm::sin(fragment.originalPos.x * 7.0f) + glm::cos(fragment.originalPos.y * 7.0f)) * 0.5f + 0.5f;

    glm::vec3 baseColor = glm::mix(glm::mix(coolColor, warmColor, heatValue), hotColor, heatValue);

    float totalNoise = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < 3; ++i) {
        float noiseValue = noiseGenerator(fragment.originalPos.x * frequency, fragment.originalPos.y * frequency, fragment.originalPos.z);
        totalNoise += amplitude * noiseValue;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    float whiteSpotThreshold = 1.5f;

    if (totalNoise > whiteSpotThreshold) {
        glm::vec3 whiteSpotColor = glm::vec3(1.0, 1.0, 0.8);
        baseColor = glm::mix(baseColor, whiteSpotColor, 0.5f);
    }

    baseColor = baseColor * fragment.intensity;
    fragment.color = Color(baseColor.r, baseColor.g, baseColor.b);

    return fragment;
}

Fragment fragmentShaderPluton(Fragment& fragment) {
    glm::vec3 sunCenter = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 fragmentToMoon = sunCenter - fragment.worldPos;
    float distanceToSun = glm::length(fragmentToMoon);
    float sunRadius = 1.0f;
    float glowFactor = glm::smoothstep(sunRadius, sunRadius * 1.2f, distanceToSun);

    glm::vec3 innerColor = glm::vec3(0.6196078431372549, 0.5372549019607843, 0.12549019607843137);
    glm::vec3 middleColor = glm::vec3(0.6196078431372549, 0.5372549019607843, 0.12549019607843137);
    glm::vec3 outerColor = glm::vec3(0.043137254901960784, 0.20784313725490197, 0.37254901960784315);

    glm::vec3 finalColor = innerColor * glowFactor + middleColor * (1.0f - glowFactor) + outerColor * (1.0f - glowFactor) * (1.0f - glowFactor);

    finalColor =  finalColor * fragment.intensity;

    glm::vec3 spotColor = glm::vec3(0.12549019607843137, 0.6196078431372549, 0.5843137254901961);
    float randomValue = glm::fract(glm::sin(fragment.worldPos.x * 5.0f) * 43758.5453f);
    finalColor = glm::mix(finalColor, spotColor, randomValue * 0.5f); 
    fragment.color = Color(finalColor.r, finalColor.g, finalColor.b);

    return fragment;
}

Fragment fragmentShader(Fragment& fragment, shaderType shaderType) {
    switch (shaderType) {
        case shaderType::Random:
            return fragmentShaderRandom(fragment);
        case shaderType::Earth:
            return fragmentShaderEarth(fragment);
        case shaderType::Neptune:
            return fragmentShaderNeptune(fragment);
        case shaderType::Sun:
            return fragmentShaderSun(fragment);
        case shaderType::Moon:
            return fragmentShaderMoon(fragment);
        case shaderType::Venus:
            return fragmentShaderVenus(fragment);
        case shaderType::Pluton:
            return fragmentShaderPluton(fragment);
        default:
        
            return fragment;
    }
}