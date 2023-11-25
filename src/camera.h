/*---------------------------------------------------------------------------
Copyright (C), 2022-2023, Sara Echeverria (bl33h)
@author Sara Echeverria
FileName: camera.h
@version: I
Creation: 29/09/2023
Last modification: 23/11/2023
*Some parts were made using the AIs Bard and ChatGPT
------------------------------------------------------------------------------*/
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
struct Camera {
glm::vec3 cameraPosition;
glm::vec3 targetPosition;
glm::vec3 upVector;

Camera(const glm::vec3& initialPosition, const glm::vec3& initialTarget, const glm::vec3& initialUp)
    : cameraPosition(initialPosition), targetPosition(initialTarget), upVector(initialUp) {}

void moveForward(float speed) {
    glm::vec3 cameraDirection = glm::normalize(targetPosition - cameraPosition);
    cameraPosition += cameraDirection * speed;
    targetPosition = cameraPosition + cameraDirection;
}

void moveBackward(float speed) {
    glm::vec3 cameraDirection = glm::normalize(targetPosition - cameraPosition);
    cameraPosition -= cameraDirection * speed;
    targetPosition = cameraPosition + cameraDirection;
}

void rotateLeft(float angle) {
    glm::vec3 cameraDirection = glm::normalize(targetPosition - cameraPosition);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), upVector);
    cameraDirection = glm::vec3(rotation * glm::vec4(cameraDirection, 0.0f));
    targetPosition = cameraPosition + cameraDirection;
}

void rotateRight(float angle) {
    glm::vec3 cameraDirection = glm::normalize(targetPosition - cameraPosition);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-angle), upVector);
    cameraDirection = glm::vec3(rotation * glm::vec4(cameraDirection, 0.0f));
    targetPosition = cameraPosition + cameraDirection;
}

glm::mat4 getViewMatrix() const {
    return glm::lookAt(cameraPosition, targetPosition, upVector);
}
};
