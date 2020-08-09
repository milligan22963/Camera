/**
 * App.cpp
 */

#include "App.h"

namespace afm
{
    namespace graphic
    {
        IAppSPtr App::GetApplication()
        {
            static IAppSPtr pAppInstance = nullptr;

            if (pAppInstance == nullptr) {
                pAppInstance = std::make_shared<App>();
            }

            return pAppInstance;
        }

        App::App()
        {
        }

        App::~App()
        {
            Shutdown();
        }

        bool App::Initialize(const std::string &glade_file)
        {
            bool success = false;

            m_p_builder = gtk_builder_new();
            if (m_p_builder != nullptr)
            {
                gtk_builder_add_from_file(m_p_builder, glade_file.c_str(), nullptr);
                gtk_builder_connect_signals(m_p_builder, nullptr);

                success = true;
            }
            return success;
        }

        void App::Shutdown()
        {
            if (m_p_builder != nullptr) {
                g_object_unref(m_p_builder);

                m_p_builder = nullptr;
            }
        }

        void App::Run(IAfmWindowSPtr pMainWindow)
        {
            if (pMainWindow->Initialize(m_p_builder) == true) {
                pMainWindow->Show();
            }

            gtk_main();
        }

        void App::ShowWindow(IAfmWindowSPtr pWindow)
        {
            if (pWindow->Initialize(m_p_builder) == true) {
                pWindow->Show();
            }
        }
    }
}