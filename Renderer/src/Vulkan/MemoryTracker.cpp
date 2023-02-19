#include "MemoryTracker.h"

#include "Buffer.h"

using namespace Vulkan;

constexpr const uint32_t PREALLOCATED_ENTRIES = 32;

MemoryTracker::MemoryTracker()
{
    mBuffers.reserve(PREALLOCATED_ENTRIES);
}

MemoryTracker::~MemoryTracker()
{ }

void MemoryTracker::AddBuffer(std::unique_ptr<Buffer> && buffer)
{
    CHECK(buffer != nullptr) << "Can not register an null-buffer in the memory tracker";
    mBuffers.emplace_back(std::move(buffer));
}

void Vulkan::MemoryTracker::Flush()
{
    for (auto& buffer : mBuffers)
    {
        buffer.reset();
    }
    mBuffers.clear();
}
