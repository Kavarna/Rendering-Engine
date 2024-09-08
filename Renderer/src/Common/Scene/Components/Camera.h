#pragma once


#include <Jnrlib.h>

namespace Common
{
    class CameraUtils;
    class Entity;
}

namespace Common
{
    namespace Components
    {
        struct Base;
        struct Camera
        {
            friend class Common::CameraUtils;
        public:
            bool primary = true;
            
            Jnrlib::Float focalDistance = 0.0f;
            glm::vec2 viewportSize = glm::vec2(0.0f);
            glm::vec2 projectionSize = glm::vec2(0.0f);

            Jnrlib::Float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;

            Entity *entityPtr;

            glm::vec3 GetUpperLeftCorner() const;
            glm::vec3 GetForwardDirection() const;
            glm::vec3 GetRightDirection() const;
            glm::vec3 GetUpDirection() const;

            void Update();

        private:
            glm::vec3 forwardDirection = glm::vec3(0.0f);
            glm::vec3 rightDirection = glm::vec3(0.0f);
            glm::vec3 upDirection = glm::vec3(0.0f);

            Jnrlib::Direction upperLeftCorner = glm::vec3(0.0f);
        };
    }
}