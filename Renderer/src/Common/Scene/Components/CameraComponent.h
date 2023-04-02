#pragma once

#include <Jnrlib.h>
#include <entt/entt.hpp>

#include "Ray.h"
#include "Common/Scene/Entity.h"
#include "CreateInfo/CameraCreateInfo.h"

namespace Common::Components
{
    struct Camera
    {
        glm::vec3 position = glm::vec3(0.0f);

        std::string name = "Camera";

        glm::vec3 forwardDirection = glm::vec3(0.0f);
        glm::vec3 rightDirection = glm::vec3(0.0f);
        glm::vec3 upDirection = glm::vec3(0.0f);

        Jnrlib::Float focalDistance = 0.0f, fieldOfView = 0.0f, aspectRatio = 0.0f;
        Jnrlib::Float viewportWidth = 0.0f, viewportHeight = 0.0f;

        Jnrlib::Float roll, pitch, yaw;

        glm::mat4x4 view;
        glm::mat4x4 projection;

        Jnrlib::Float projectionWidth, projectionHeight;

        Jnrlib::Direction upperLeftCorner;


        Ray GetRayForPixel(uint32_t x, uint32_t y) const;

        void MoveForward(float amount);
        void MoveBackward(float amount);
        void StrafeRight(float amount);
        void StrafeLeft(float amount);

        void Roll(float amount);
        void Yaw(float amount);
        void Pitch(float amount);

        glm::mat4x4 const& GetView() const;
        glm::mat4x4 const& GetProjection() const;

        uint32_t GetDirtyFrames() const;

        void CalculateVectors();
        void CalculateViewMatrix();
        void CalculateProjectionMatrix();
        void CalculateMatrices();
        void CalculateUpperLeftCorner();

    };

    std::unique_ptr<Entity> InitCameraEntity(
        entt::registry& registry, CreateInfo::Camera const& info, Entity* parent, uint32_t index);
}
