/**
 * AfmWindow.cpp
 */

#include "AfmWindow.h"

namespace afm
{
    namespace graphic
    {
        AfmWindow::AfmWindow()
        {

        }

        AfmWindow::~AfmWindow()
        {
            Shutdown();
        }

        bool AfmWindow::Initialize(GtkBuilder *p_builder)
        {
            bool success = false;

            if (m_window_name.size() > 0) {
                if (p_builder != nullptr) {
                    m_p_window = GTK_WIDGET(gtk_builder_get_object(p_builder, m_window_name.c_str()));
                    if (m_p_window != nullptr) {
                        success = true;
                    }
                }
            }

            return success;
        }

        void AfmWindow::Shutdown()
        {
            if (m_p_window != nullptr) {
                gtk_widget_destroy(m_p_window);
                m_p_window = nullptr;
            }
        }

        void AfmWindow::Show()
        {
            if (m_p_window != nullptr) {
                gtk_widget_show(m_p_window);
            }
        }

        void AfmWindow::ShowCursor(GdkCursor *p_hide_cursor)
        {
            GdkWindow		*p_gdk_window = nullptr;

            p_gdk_window = gtk_widget_get_window(m_p_window);
            gdk_window_set_cursor(p_gdk_window, p_hide_cursor);
        }

        void AfmWindow::HideCursor()
        {
            GdkDisplay		*p_display = nullptr;
            GdkCursor		*p_hide_cursor = nullptr;

            // Hide the curssor
            p_display = gdk_display_get_default();
            if (p_display != nullptr) {
                p_hide_cursor = gdk_cursor_new_for_display(p_display, GDK_BLANK_CURSOR);

                ShowCursor(p_hide_cursor);
            }
        }

        void AfmWindow::MoveCursor(uint16_t x, uint16_t y)
        {
            Display *p_display = nullptr;
            ::Window x_root_window;
            
            p_display = XOpenDisplay(0);
            if (p_display != nullptr)
            {
                x_root_window = XRootWindow(p_display, 0);
                XSelectInput(p_display, x_root_window, KeyReleaseMask);
                XWarpPointer(p_display, None, x_root_window, 0, 0, 0, 0, x, y);
                XFlush(p_display);
            }
        }

        void AfmWindow::SetWindowSize(uint16_t width, uint16_t height)
        {
            m_width = width;
            m_height = height;
        }
    }
}