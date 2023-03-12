#include "JnrSerializer.h"
#include "MathHelpers.h"
#include "FileHelpers.h"

#include <glog/logging.h>

using namespace Jnrlib;

static constexpr const uint32_t JNR_FILE_MAGIC = '%RNJ';
static constexpr const uint32_t JNR_FILE_ALIGNMENT = 256;
static constexpr const uint32_t JNR_FILE_VERSION = 1;
static constexpr const uint32_t JNR_PER_STRUCTURE_INFO_ELEMENTS = 2;

JnrSerializer::JnrSerializer(std::string const& path) :
    mDumpPath(path)
{
}

void JnrSerializer::AddStructure(StructureType const& structureType)
{
    mStructures.push_back(structureType);
}

void JnrSerializer::Flush()
{
    uint64_t fileSize = EstimateFileSize();
    std::vector<unsigned char> content(fileSize, 0);

    uint32_t* contentU32 = (uint32_t*)content.data();
    
    *contentU32 = JNR_FILE_MAGIC;
    contentU32++;

    *contentU32 = JNR_FILE_VERSION;
    contentU32++;

    *contentU32 = (uint32_t)mStructures.size();
    contentU32++;

    std::vector<uint32_t> offsets;
    std::vector<uint32_t> structureTypes;
    std::vector<uint32_t> sizes;
    offsets.reserve(mStructures.size());
    structureTypes.reserve(mStructures.size());
    sizes.reserve(mStructures.size());

    uint64_t firstOffset = (unsigned char*)contentU32 + (sizeof(uint32_t) * (uint32_t)mStructures.size() * JNR_PER_STRUCTURE_INFO_ELEMENTS) - content.data();
    firstOffset = AlignUp(firstOffset, JNR_FILE_ALIGNMENT);

    uint32_t lastOffset = (uint32_t)firstOffset;
    for (uint32_t i = 0; i < mStructures.size(); ++i)
    {
        auto offset = lastOffset;
        uint32_t size = 0;
        std::visit([&](auto structure)
        {
            size = structure.EstimateSize();
        }, mStructures[i]);
        offsets.push_back(offset);
        structureTypes.push_back((uint32_t)mStructures[i].index());
        sizes.push_back((uint32_t)size);

        lastOffset = offset + size;
        lastOffset = AlignUp(lastOffset, JNR_FILE_ALIGNMENT);
    }

    for (uint32_t i = 0; i < mStructures.size(); ++i)
    {
        *contentU32 = offsets[i];
        contentU32++;
        *contentU32 = structureTypes[i];
        contentU32++;
        *contentU32 = sizes[i];
        contentU32++;
    }

    for (uint32_t i = 0; i < mStructures.size(); ++i)
    {
        auto contentPtr = content.data() + offsets[i];
        std::visit([contentPtr](auto structure)
        {
            structure.Dump(contentPtr);
        }, mStructures[i]);
    }

    DumpWholeFile(mDumpPath, content);
}

uint32_t JnrSerializer::EstimateFileSize()
{
    uint32_t sum = sizeof(JNR_FILE_MAGIC) +
        sizeof(JNR_FILE_VERSION) +
        sizeof(uint32_t) /* section count */ + 
        sizeof(uint32_t) * (uint32_t)mStructures.size() * JNR_PER_STRUCTURE_INFO_ELEMENTS /* Per size info */;
    sum = AlignUp(sum, JNR_FILE_ALIGNMENT);
    for (uint32_t i = 0; i < mStructures.size(); ++i)
    {
        std::visit([&sum](auto structure)
        {
            sum += structure.EstimateSize();
        }, mStructures[i]);
        sum = AlignUp(sum, JNR_FILE_ALIGNMENT);
    }

    return sum;
}

JnrDeserializer::JnrDeserializer(std::string const& path) :
    mReadPath(path)
{ }

void JnrDeserializer::Read()
{
    auto content = ReadWholeFile(mReadPath, false);
    if (content.size() == 0)
    {
        return;
    }

    uint32_t* contentU32 = (uint32_t*)content.data();
    CHECK(*contentU32 == JNR_FILE_MAGIC) << "Reading JNR file with invalid magic";
    contentU32++;

    CHECK(*contentU32 == JNR_FILE_VERSION) << "Reading JNR file with invalid version (" << *contentU32 << "). Expecting " << JNR_FILE_VERSION;
    contentU32++;

    uint32_t numberOfStructures = *contentU32;
    contentU32++;

    std::vector<uint32_t> offsets;
    std::vector<uint32_t> structureTypes;
    std::vector<uint32_t> sizes;
    
    offsets.resize(numberOfStructures);
    structureTypes.resize(numberOfStructures);
    sizes.resize(numberOfStructures);
    
    for (uint32_t i = 0; i < numberOfStructures; ++i)
    {
        offsets[i] = *contentU32;
        contentU32++;
        structureTypes[i] = *contentU32;
        contentU32++;
        sizes[i] = *contentU32;
        contentU32++;
    }

    mStructures.resize(numberOfStructures);
    for (uint32_t i = 0; i < numberOfStructures; ++i)
    {
        auto contentPtr = (unsigned char*)content.data() + offsets[i];
        switch (structureTypes[i])
        {
            case 0:
                mStructures[i] = InfoWindowsV1{};
                break;
            case 1:
                mStructures[i] = InfoMainWindowV1{};
            default:
                break;
        }
        std::visit([contentPtr](auto& structure)
        {
            structure.Load(contentPtr);
        }, mStructures[i]);
    }
}

uint32_t Jnrlib::JnrDeserializer::GetNumStructures() const
{
    return (uint32_t)mStructures.size();
}

JnrDeserializer::StructureType JnrDeserializer::GetStructure(uint32_t index)
{
    CHECK(index < mStructures.size()) << "Unable to read structure at index " << index;

    return mStructures[index];
}
