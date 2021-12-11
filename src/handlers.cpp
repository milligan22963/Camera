#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include "App.h"
#include "util.h"
#include "handlers.h"
#include "AboutDialog.h"
#include "PreferencesDialog.h"

afm::graphic::IAfmWindowSPtr g_about_dialog = nullptr;
afm::graphic::IAfmWindowSPtr g_prefrences_dialog = nullptr;

extern "C"
{
  void OnAbout()
  {
      g_about_dialog = std::make_shared<afm::graphic::AboutDialog>();

      afm::graphic::IAppSPtr pApp = afm::graphic::App::GetApplication();

      pApp->ShowWindow(g_about_dialog);
  }

  void OnAboutOK(GtkWidget *p_widget, gpointer data)
  {
    if (g_about_dialog != nullptr) {
        g_about_dialog->Shutdown();
        g_about_dialog = nullptr;
    }
  }

  void OnOK(GtkWidget *p_widget, gpointer data)
  {
      if (g_prefrences_dialog != nullptr) {
          g_prefrences_dialog->Shutdown();
          g_prefrences_dialog = nullptr;
      }
  }

  void OnCancel(GtkWidget *p_widget, gpointer data)
  {
      if (g_prefrences_dialog != nullptr) {
          g_prefrences_dialog->Shutdown();
          g_prefrences_dialog = nullptr;
      }
  }

  void OnSetDefaults()
  {
  }

  void OnPreferences()
  {
      g_prefrences_dialog = std::make_shared<afm::graphic::PreferencesDialog>();

      afm::graphic::IAppSPtr pApp = afm::graphic::App::GetApplication();

      pApp->ShowWindow(g_prefrences_dialog);
  }

  void OnWindowMainDestroy()
  {
      gtk_main_quit();
  }
}
