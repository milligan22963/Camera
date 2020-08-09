/**
 * Handlers
 */

#ifndef _H_HANDLERS
#define _H_HANDLERS

#include <string>

const std::string glade_file = "resources/Camera.glade";

extern "C"
{
  void OnAbout();
  void OnAboutOK(GtkWidget *p_widget, gpointer data);
  void OnOK(GtkWidget *p_widget, gpointer data);
  void OnCancel(GtkWidget *p_widget, gpointer data);
  void OnPreferences();
  void OnWindowMainDestroy();
}
#endif
