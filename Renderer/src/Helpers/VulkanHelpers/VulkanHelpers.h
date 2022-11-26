#pragma once

#include "Exceptions.h"

#define ThrowIfFailed(res) {\
auto _res = res; \
if (_res != VK_SUCCESS) \
{\
throw Jnrlib::Exceptions::VulkanException(_res);\
}\
}
