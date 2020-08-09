/**
 * Window.h
 */

#ifndef _H_WINDOW
#define _H_WINDOW

#include "IAfmWindow.h"

namespace afm
{
    namespace graphic
    {
        class AfmWindow : public IAfmWindow
        {
            public:
                AfmWindow();
                virtual ~AfmWindow();

                virtual bool Initialize(GtkBuilder *p_builder) override;
                virtual void Shutdown() override;
                virtual void Show() override;

                virtual void ShowCursor(GdkCursor *p_hide_cursor) override;
                virtual void HideCursor() override;
                virtual void MoveCursor(uint16_t x, uint16_t y) override;

                virtual void SetWindowSize(uint16_t width, uint16_t height) override;

            protected:
                void SetWindowName(const std::string &window_name) { m_window_name = window_name; }

            private:
                GtkBuilder  *m_p_builder = nullptr;
                GtkWidget   *m_p_window = nullptr;
                std::string m_window_name;
                uint16_t    m_width;
                uint16_t    m_height;
        };
    }
}
#endif