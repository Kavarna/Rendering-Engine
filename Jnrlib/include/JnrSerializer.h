#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace Jnrlib
{
    struct InfoWindowsV1
    {
        struct WindowInfo
        {
            uint32_t isOpen;
        };
        std::vector<WindowInfo> windows;

        void Dump(unsigned char* content)
        {
            uint32_t* contentU32 = (uint32_t*)content;
            *contentU32 = (uint32_t)windows.size();
            contentU32++;
            for (uint32_t i = 0; i < windows.size(); ++i)
            {
                *contentU32 = windows[i].isOpen ? 1 : 0;
                contentU32++;
            }
        }

        void Load(unsigned char* content)
        {
            uint32_t* contentU32 = (uint32_t*)content;
            uint32_t count = *contentU32;
            contentU32++;
            windows.resize(count);
            for (uint32_t i = 0; i < windows.size(); ++i)
            {
                windows[i].isOpen = (*contentU32) == 1 ? true : false;
                contentU32++;
            }
        }

        uint32_t EstimateSize()
        {
            return (uint32_t)windows.size() * sizeof(WindowInfo);
        };
    };

    struct InfoMainWindowV1
    {
        uint32_t width = 0;
        uint32_t height = 0;

        void Dump(unsigned char* content)
        {
            uint32_t* contentU32 = (uint32_t*)content;
            *contentU32 = width;
            contentU32++;
            *contentU32 = height;
        }

        void Load(unsigned char* content)
        {
            uint32_t* contentU32 = (uint32_t*)content;
            width = *contentU32;
            contentU32++;
            height = *contentU32;
        }

        uint32_t EstimateSize()
        {
            return sizeof(width) + sizeof(height);
        }
    };

    /**
    * JNR file structure:
    * <MAGIC><FILE_VERSION><NUMBER_OF_STRUCTURES><<offsests><structure_types><sizes> * NUMBER_OF_STRUCTURES>
    * At offset there's one of the above structures
    */

    class JnrSerializer
    {
    public:
        using StructureType = std::variant<InfoWindowsV1, InfoMainWindowV1>;;

        JnrSerializer(std::string const& path);
        ~JnrSerializer() = default;

        void AddStructure(StructureType const&);

        void Flush();

    private:
        uint32_t EstimateFileSize();

    private:
        std::string mDumpPath;

        std::vector<StructureType> mStructures;
    };

    class JnrDeserializer
    {
    public:
        using StructureType = std::variant<InfoWindowsV1, InfoMainWindowV1>;

        JnrDeserializer(std::string const& path);
        ~JnrDeserializer() = default;

    public:
        void Read();
        
        uint32_t GetNumStructures() const;
        StructureType GetStructure(uint32_t index = 0);

    private:
        std::string mReadPath;

        std::vector<StructureType> mStructures;

    };
}


