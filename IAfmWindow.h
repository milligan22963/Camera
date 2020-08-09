/**
 * IWindow.h
 */

#ifndef _H_IWINDOW
#define _H_IWINDOW

#include <cstdint>
#include <memory>

#include <gtk/gtk.h>
#include <X11/Xlib.h>

namespace afm
{
    namespace graphic
    {
        class IAfmWindow
        {
            public:
                virtual ~IAfmWindow() {}

                virtual bool Initialize(GtkBuilder *p_builder) = 0;
                virtual void Shutdown() = 0;
                virtual void Show() = 0;

                virtual void ShowCursor(GdkCursor *p_hide_cursor) = 0;
                virtual void HideCursor() = 0;
                virtual void MoveCursor(uint16_t x, uint16_t y) = 0;

                virtual void SetWindowSize(uint16_t width, uint16_t height) = 0;
        };

        using IAfmWindowSPtr = std::shared_ptr<IAfmWindow>;
    }
}
#endif