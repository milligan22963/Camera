/**
 *  Utilities associated w/ the camera
 */

#ifndef _H_CAMERA_UTILITIES
#define _H_CAMERA_UTILITIES

#include <string>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

GtkWidget*  CreateWindowWithGlade(const std::string &glade_file, const std::string &win_name);
void        HideCursor(GtkWidget *p_window);
void        MoveCursor(int x, int y);

#endif
