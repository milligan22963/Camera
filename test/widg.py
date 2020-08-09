#!/usr/bin/python3

from PIL import Image
#import cairo
from picamera import PiCamera

from time import sleep
import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GdkPixbuf
from gpiozero import Button
import sqlite3

def CameraPreviewOn():
    global camera

    preview_image = self._builder.get_object("m_previewImage")
    self._last_position = preview_image.get_window().get_frame_extents()
    
    camera.start_preview()
    camera.preview.fullscreen = False
    camera.preview.window = (self._last_position.x, self._last_position.y, self._last_position.width, self._last_position.height)

def CameraPreviewOff():
    global camera

    camera.stop_preview()

def NextPicture():
    global camera

    camera.capture('/tmp/file')

class Settings:
    _builder = None
    _camera = None
    _iso = None
    _ev_comp = None
    _awb = None
    _sharpness = None
    _exposure = None
    _ucoloreffect = None
    _vcoloreffect = None
    _effects = None
    _metering = None
    _dynamicrange = None
    _analoggain = None
    _auto_iso = None
    _auto_exposure = None
    _auto_awb = None
    _auto_metering = None
    _disable_effects = None
    _disable_dynamic_range = None

    def __init__(self, builder, camera):
      self._builder = builder
      self._camera = camera
      self._iso = builder.get_object("m_iso")
      self._ev_comp = builder.get_object("m_evcomp")
      self._awb = builder.get_object("m_awb")
      self._sharpness = builder.get_object("m_sharpness")
      self._exposure = builder.get_object("m_exposure")
      self._ucoloreffect = builder.get_object("m_ucoloreffect")
      self._vcoloreffect = builder.get_object("m_vcoloreffect")
      self._effects = builder.get_object("m_effects")
      self._metering = builder.get_object("m_metering")
      self._dynamicrange = builder.get_object("m_dynamicrange")
      self._analoggain = builder.get_object("m_analoggain")
      self._auto_iso = builder.get_object("m_auto_iso")
      self._auto_exposure = builder.get_object("m_auto_exposure")
      self._auto_awb = builder.get_object("m_auto_awb")
      self._auto_metering = builder.get_object("m_auto_metering")
      self._disable_effects = builder.get_object("m_disable_effects")
      self._disable_dynamic_range = builder.get_object("m_disable_dynamic_range")

class Handler:
    _builder = None
    _current_window = None
    _iso_spinner = None
    _last_position = None
    _settings = None

    def __init__(self, builder):
      self._builder = builder
      self._iso_spinner = self._builder.get_object("m_iso")

    def onDestroy(self, *args):
        Gtk.main_quit()

    def OnAbout(self, button):
      self._current_window = Gtk.AboutDialog()
      self._current_window.set_authors(["D.W. Milligan"])
      self._current_window.set_website("www.afmsoftware.com")
      self._current_window.set_version("1.0.0")
      self._current_window.set_copyright("D.W. Milligan")
      self._current_window.set_comments("Built on Raspberry PI utilising concepts from raspistill and the associated Broadcom drivers.")
      self._current_window.set_modal(True)
      self._current_window.present()

    def OnPreferences(self, button):
      global camera

      self._settings = Settings(self._builder, camera)
      self._current_window = self._builder.get_object("PreferencesDlg")
      self._current_window.set_modal(True)
      self._current_window.present()

    def OnAboutOK(self, button):
        self._current_window.close()

    def OnSetDefaults(self, button):
        print("SetDefaults")

    def OnCancel(self, button):
      self._current_window.close()

    def OnOK(self, button):
      self._current_window.close()

    def OnMainShown(self, *args):
      print("OnMainShown")
      
    def OnWindowMainDestroy(self, *args):
      global camera
      
      camera.stop_preview()
      Gtk.main_quit()

camera = None
database_conn = sqlite3.connect('camera.db')

LiveView = Button(22)
TakePicture = Button(26)

LiveView.when_pressed = CameraPreviewOn
LiveView.when_released = CameraPreviewOff
TakePicture.when_pressed = NextPicture

def main():
  global camera
  builder = Gtk.Builder()
  builder.add_from_file("Camera.glade")
  builder.connect_signals(Handler(builder))

  pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size('/home/daniel/Pictures/buddy.jpg', 640, 460)
  #pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size('/home/pi/Pictures/buddy.jpg', 640, 460)

  preview_image = builder.get_object("m_previewImage")
  preview_image.set_from_pixbuf(pixbuf)

  window = builder.get_object("Camera")
  window.present()

  camera = PiCamera()

  Gtk.main()

if __name__ == "__main__":
  main()
