/**
 * App.h
 */

#ifndef _H_APP
#define _H_APP

#include "IApp.h"

namespace afm
{
    namespace graphic
    {
        class App : public IApp
        {
            public:
                App();
                virtual ~App();

                static IAppSPtr GetApplication();

                virtual bool Initialize(const std::string &glade_file) override;
                virtual void Shutdown() override;
                virtual void Run() override;
                virtual void ShowWindow(IAfmWindowSPtr pWindow) override;

            private:
                GtkBuilder  *m_p_builder = nullptr;
        };
    }
}
#endif