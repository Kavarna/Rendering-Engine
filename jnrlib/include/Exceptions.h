#pragma once

#include <string>
#include <exception>


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

            virtual const char* what() const override
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
    }
}

