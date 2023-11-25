/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: barycentric.cpp
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#include "fragment.h"

SDL_Surface* currentTexture = nullptr;

extern SDL_Surface* currentTexture;
bool loadTexture(const std::string& filename);
Color getPixelFromTexture(float u, float v);
glm::vec3 getNormalFromTexture(float u, float v);
bool loadTexture(const std::string& filename) {
    if (currentTexture != nullptr) {
        SDL_FreeSurface(currentTexture);
        currentTexture = nullptr;
    }
    return true;
}

Color getPixelFromTexture(float u, float v) {
    int texWidth = currentTexture->w;
    int texHeight = currentTexture->h;

    int x = static_cast<int>(u * texWidth);
    int y = static_cast<int>((1.0f - v) * texHeight);

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= texWidth) x = texWidth - 1;
    if (y >= texHeight) y = texHeight - 1;

    Uint32 pixel = ((Uint32*)currentTexture->pixels)[y * currentTexture->pitch / sizeof(Uint32) + x];
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, currentTexture->format, &r, &g, &b, &a);
    
    return Color(r, g, b, a);
}

glm::vec3 getNormalFromTexture(float u, float v) {
    int texWidth = currentTexture->w;
    int texHeight = currentTexture->h;
    int x = static_cast<int>(u * texWidth);
    int y = static_cast<int>((1.0f - v) * texHeight);

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= texWidth) x = texWidth - 1;
    if (y >= texHeight) y = texHeight - 1;

    Uint32 pixel = ((Uint32*)currentTexture->pixels)[y * currentTexture->pitch / sizeof(Uint32) + x];
    
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, currentTexture->format, &r, &g, &b, &a);

    float xNormal = (r / 255.0f) * 2.0f - 1.0f;
    float yNormal = (g / 255.0f) * 2.0f - 1.0f;
    float zNormal = (b / 255.0f) * 2.0f - 1.0f;

    return glm::vec3(xNormal, yNormal, zNormal);
}
