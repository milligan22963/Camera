/**
 *  Utilities associated w/ the camera
 */

#ifndef _H_CAMERA_UTILITIES
#define _H_CAMERA_UTILITIES

#include <memory>
#include <fstream>
#include <string>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

GtkWidget*  CreateWindowWithGlade(const std::string &glade_file, const std::string &win_name);
void        HideCursor(GtkWidget *p_window);
void        MoveCursor(int x, int y);

class Logger {
    public:
        enum LOG_LEVEL {
            LOG_LEVEL_DEBUG         = 0x00,
            LOG_LEVEL_TRACE,
            LOG_LEVEL_INFO,
            LOG_LEVEL_IMPORTANT,
            LOG_LEVEL_WARNING,
            LOG_LEVEL_ERROR,
            LOG_LEVEL_FATAL,
            END_LOG_LEVELS
        };

        enum LOG_TYPE {
            LOG_CONSOLE = 0x01,
            LOG_FILE = 0x02,
            LOG_SYSLOG = 0x04,
            END_LOG_TYPES
        };
        
        Logger();
        virtual ~Logger();

        static std::shared_ptr<Logger> GetInstance() {
            if (m_logInstance != nullptr) {
                m_logInstance = std::make_shared<Logger>();
            }
            return m_logInstance;
        }
        bool InitializeLog(LOG_TYPE type, const std::string &name, LOG_LEVEL minimumLevel);
        void Log(const std::string &message, LOG_LEVEL level);

    private:
        static std::shared_ptr<Logger> m_logInstance;
        LOG_LEVEL m_logLevel;
        LOG_TYPE m_logType;
        std::fstream m_logFile;
        std::string m_name;
};

using LogSPtr = std::shared_ptr<Logger>;

#endif
