/**
 *  Utilities associated w/ the camera
 */

#include <iostream>
#include "util.h"

GtkWidget* CreateWindowWithGlade(const std::string &glade_file, const std::string &win_name)
{
    GtkBuilder  *p_builder = nullptr;
    GtkWidget   *p_window = nullptr;

    p_builder = gtk_builder_new();
    if (p_builder != nullptr)
    {
        gtk_builder_add_from_file(p_builder, glade_file.c_str(), nullptr);

        p_window = GTK_WIDGET(gtk_builder_get_object(p_builder, win_name.c_str()));
        gtk_builder_connect_signals(p_builder, nullptr);

        g_object_unref(p_builder);

        // Move cursor to off screen
        MoveCursor(800, 480);

        // show the window
        gtk_widget_show(p_window);

        // Hide the cursor
        HideCursor(p_window);
    }    
    return p_window;
}

void HideCursor(GtkWidget *p_window)
{
    GdkDisplay		*p_display = nullptr;
    GdkCursor		*p_hide_cursor = nullptr;
    GdkWindow		*p_gdk_window = nullptr;

    // Hide the curssor
    p_display = gdk_display_get_default();
    if (p_display != nullptr)
    {
        p_hide_cursor = gdk_cursor_new_for_display(p_display, GDK_BLANK_CURSOR);
        p_gdk_window = gtk_widget_get_window(p_window);
        gdk_window_set_cursor(p_gdk_window, p_hide_cursor);
    }
}

void MoveCursor(int x, int y)
{
    Display *p_display = nullptr;
    Window x_root_window;
	
    p_display = XOpenDisplay(0);
    if (p_display != nullptr)
    {
        x_root_window = XRootWindow(p_display, 0);
        XSelectInput(p_display, x_root_window, KeyReleaseMask);
        XWarpPointer(p_display, None, x_root_window, 0, 0, 0, 0, x, y);
        XFlush(p_display);
    }
}

const std::string LOG_LEVEL_STRINGS[]={
    "DEBUG: ",
    "TRACE: ",
    "INFORMATION: ",
    "IMPORTANT: ",
    "WARNING: ",
    "ERROR: ",
    "FATAL: ",
    "INVALID"
};

Log::Log()
    : m_logLevel(LOG_LEVEL::LOG_LEVEL_ERROR)
    , m_logType(LOG_TYPE::LOG_SYSLOG)
{
    // if anything else is needed
}

Log::~Log()
{
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

bool Log::InitializeLog(LOG_TYPE type, const std::string &name, LOG_LEVEL minimumLevel)
{
    bool success = true;

    m_logType = type;
    m_logLevel = minimumLevel;
    m_name = name;

    if (m_logType & LOG_TYPE::LOG_FILE) {
        m_logFile.open(m_name, iso_base::ate | iso_base::out);
        if (!m_logFile.is_open()) {
            success = false;
        }
    }
    return success;
}

void Log::Log(const std::string &message, LOG_LEVEL level)
{
    if (level > m_logLevel) {
        if (level > LOG_LEVEL::END_LOG_LEVELS) {
            level = LOG_LEVEL::END_LOG_LEVELS;
        }
        if (m_logType & LOG_TYPE::LOG_FILE) {
            m_logFile << LOG_LEVEL_STRINGS[level] << message;
        }
        if (m_logType & LOG_TYPE::LOG_CONSOLE) {
            std::cout << LOG_LEVEL_STRINGS[level] << message;
        }
        if (m_logType & LOG_TYPE::LOG_SYSLOG) {
            //
        }
    }
}
