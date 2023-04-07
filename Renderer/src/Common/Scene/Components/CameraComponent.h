#pragma once


#include <Jnrlib.h>

namespace Common
{
    class CameraUtils;
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
            Camera(Base& base) :
                baseComponent(base)
            { }

            bool primary = true;

            glm::vec3 forwardDirection = glm::vec3(0.0f);
            glm::vec3 rightDirection = glm::vec3(0.0f);
            glm::vec3 upDirection = glm::vec3(0.0f);
            
            Jnrlib::Float focalDistance = 0.0f, fieldOfView = 0.0f, aspectRatio = 0.0f;
            glm::vec2 viewportSize = glm::vec2(0.0f);
            glm::vec2 projectionSize = glm::vec2(0.0f);

            Jnrlib::Float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;

            glm::mat4x4 view;
            glm::mat4x4 projection;

            Jnrlib::Direction upperLeftCorner = glm::vec3(0.0f);

        protected:
            /* Dependencies */
            Base& baseComponent;

        };
    }
}