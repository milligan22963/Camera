/**
 * MainWindow.cpp
 */

#include "MainWindow.h"

namespace afm
{
    namespace graphic
    {
        const std::string main_window_name = "Camera";

        MainWindow::MainWindow()
            : AfmWindow()
        {
            SetWindowName(main_window_name);
        }
    }
}
