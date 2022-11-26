#pragma once

#include <string>
#include <exception>

#include "TypeMatching.h"

namespace Jnrlib
{
    namespace Exceptions
    {
        class JNRException : public std::exception
        {
            std::string message;
        public:
            JNRException(std::string const& msg) : message(msg)
            { };

            virtual const char* what() const noexcept override 
            {
                return message.c_str();
            }
        };

        class TaskNotFound : public JNRException
        {
        public:
            TaskNotFound(uint64_t id) : JNRException("Could not find task with ID: " + std::to_string(id))
            { };
        };

        class FieldNotFound : public JNRException
        {
        public:
            FieldNotFound(std::string const& field, std::string const& type = "") :
                JNRException("Field not found: " + field + (type.size() == 0 ? "" : (" of type " + type)))
            { };
        };

        class SingletoneNotUniqueAttempt : public JNRException
        {
        public:
            SingletoneNotUniqueAttempt() : 
                JNRException("Singletone attempts not to be unique")
            {}
        };

        class SingletoneNotCreated : public JNRException
        {
        public:
            SingletoneNotCreated() : 
                JNRException("Singletone was not created")
            {}
        };

        class ImpossibleToGetHere : public JNRException
        {
        public:
            ImpossibleToGetHere(const std::string& e) :
                JNRException("How the hell did you get here?: " + e)
            { }
        };

        class VulkanException : public JNRException
        {
        public:
            VulkanException(uint32_t errorCode) :
                JNRException("Vulkan function failed with error code " + Jnrlib::to_string(errorCode))
            { }
        };
    }
}

