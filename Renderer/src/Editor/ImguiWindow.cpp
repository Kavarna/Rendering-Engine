#include "ImguiWindow.h"

Editor::ImguiWindow::ImguiWindow()
{ }

Editor::ImguiWindow::~ImguiWindow()
{ }

void Editor::ImguiWindow::OnImguiRender()
{
    if (mIsOpen)
        OnRender();
}

