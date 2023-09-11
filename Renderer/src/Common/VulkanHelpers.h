#pragma once

#include "Exceptions.h"

#include <string_view>
#include <vulkan/vk_enum_string_helper.h>

namespace Jnrlib
{
namespace Exceptions
{
class VulkanException : public JNRException
{
public:
    inline VulkanException(std::string const &function, VkResult res)
        : JNRException("Vulkan " + function + " failed with error code " + Jnrlib::to_string((uint32_t)res) + ": " +
                       string_VkResult(res))
    {
    }
};
} // namespace Exceptions
} // namespace Jnrlib

#define ThrowIfFailed(res)                                                                                             \
    {                                                                                                                  \
        auto _res = res;                                                                                               \
        if (_res != VK_SUCCESS)                                                                                        \
        {                                                                                                              \
            throw Jnrlib::Exceptions::VulkanException(#res, _res);                                                     \
        }                                                                                                              \
    }
