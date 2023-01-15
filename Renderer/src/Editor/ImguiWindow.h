#pragma once

#include <string>


namespace Editor
{
    class ImguiWindow
    {
    public:
        ImguiWindow();
        virtual ~ImguiWindow();

        void OnImguiRender();

    public:
        bool mIsOpen = false;

    protected:
        virtual void OnRender() = 0;
        

    };
}