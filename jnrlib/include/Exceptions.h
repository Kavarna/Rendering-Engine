#pragma once

#include <string>
#include <exception>


namespace Jnrlib
{
    namespace Exceptions
    {
        class TaskNotFound : public std::exception
        {
            std::string message;
        public:
            TaskNotFound(std::string const& msg) : message(msg)
            { };

            virtual const char* what() const override
            {
                return message.c_str();
            }
        };
    }
}

