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
            JNRException(std::string const& msg, const char *file, uint32_t line) : message(msg + " at line " + std::to_string(line) + " in file " + file)
            { };

            virtual const char* what() const noexcept override 
            {
                return message.c_str();
            }
        };

        class TaskNotFound : public JNRException
        {
        public:
            TaskNotFound(uint64_t id, const char *file, uint32_t line) : JNRException("Could not find task with ID: " + std::to_string(id), file, line)
            { };
        };

        class FieldNotFound : public JNRException
        {
        public:
            FieldNotFound(std::string const& field, std::string const& type, const char *file, uint32_t line) :
                JNRException("Field not found: " + field + (type.size() == 0 ? "" : (" of type " + type)), file, line)
            { };
        };

        class SingletoneNotUniqueAttempt : public JNRException
        {
        public:
            SingletoneNotUniqueAttempt(const char *file, uint32_t line) :
                JNRException("Singletone attempts not to be unique", file, line)
            {}
        };

        class SingletoneNotCreated : public JNRException
        {
        public:
            SingletoneNotCreated(const char *file, uint32_t line) :
                JNRException("Singletone was not created", file, line)
            {}
        };

        class ImpossibleToGetHere : public JNRException
        {
        public:
            ImpossibleToGetHere(const std::string& e, const char *file, uint32_t line) :
                JNRException("How the hell did you get here?: " + e, file, line)
            { }
        };
    }
}

