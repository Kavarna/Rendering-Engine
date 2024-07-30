#pragma once

#include "Exceptions.h"

#include <vulkan/vk_enum_string_helper.h>

namespace Jnrlib
{
    namespace Exceptions
    {
        class VulkanException : public JNRException
        {
        public:
            inline VulkanException(VkResult res, const char* file, uint32_t line) :
                JNRException("Vulkan function failed with error code " +
                             Jnrlib::to_string((uint32_t)res) + ": " + string_VkResult(res), file, line)
            { }
        };
    }
}

#define ThrowIfFailed(res) {\
auto _res = res; \
if (_res != VK_SUCCESS) \
{\
throw Jnrlib::Exceptions::VulkanException(_res, __FILE__, __LINE__);\
}\
}
