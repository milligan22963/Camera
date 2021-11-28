/**
 * IApp.h
 */

#ifndef _H_IAPP
#define _H_IAPP

#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include <string>

#include "IAfmWindow.h"

namespace afm
{
    namespace graphic
    {
        class IApp
        {
            public:
                virtual ~IApp() {}

                virtual bool Initialize(const std::string &glade_file) = 0;
                virtual void Shutdown() = 0;
                virtual void Run() = 0;
                virtual void ShowWindow(IAfmWindowSPtr pWindow) = 0;
        };

        using IAppSPtr = std::shared_ptr<IApp>;
    }
}
#endif