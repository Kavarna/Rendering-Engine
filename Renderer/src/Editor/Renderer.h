#pragma once

#include <Jnrlib.h>
#include "vulkan/vulkan.h"


namespace Editor
{
    class Renderer : public Jnrlib::ISingletone<Renderer>
    {
        MAKE_SINGLETONE_CAPABLE(Renderer);
    private:
        Renderer();
        ~Renderer();
    
    private:
        void InitInstance();

    private:
        VkInstance mInstance;

    };
}

