/**
 * PreferencesDialog.cpp
 */

#include "PreferencesDialog.h"

namespace afm
{
    namespace graphic
    {
        const std::string preferences_dialog = "PreferencesDlg";
        const std::string white_balance_control = "m_awb";
        const std::string ev_comp = "m_evcomp";
        const std::string dynamic_range = "m_dynamicrange";
        const std::string effects = "m_effects";
        const std::string iso = "m_iso";
        const std::string metering = "m_metering";
        const std::string sharpness = "m_sharpness";
        const std::string exposure = "m_exposure";
        const std::string analoggain = "m_analoggain";
        const std::string ucoloreffect = "m_ucoloreffect";
        const std::string vcoloreffect = "m_vcoloreffect";

        PreferencesDialog::PreferencesDialog()
            : AfmWindow()
        {
            SetWindowName(preferences_dialog);
        }

        bool PreferencesDialog::Initialize(GtkBuilder *p_builder)
        {
            bool success = AfmWindow::Initialize(p_builder);

            if (success == true) {
                GtkAdjustment *adjustment;

                m_p_awb = (GtkComboBoxText *)GTK_WIDGET(gtk_builder_get_object(p_builder, white_balance_control.c_str()));
                m_p_dynamic_range = (GtkComboBoxText *)GTK_WIDGET(gtk_builder_get_object(p_builder, white_balance_control.c_str()));
                m_p_effects = (GtkComboBoxText *)GTK_WIDGET(gtk_builder_get_object(p_builder, effects.c_str()));
                m_p_iso = (GtkComboBoxText *)GTK_WIDGET(gtk_builder_get_object(p_builder, iso.c_str()));
                m_p_metering = (GtkComboBoxText *)GTK_WIDGET(gtk_builder_get_object(p_builder, metering.c_str()));
                m_p_exposure = (GtkComboBoxText *)GTK_WIDGET(gtk_builder_get_object(p_builder, exposure.c_str()));

                m_p_ev_comp = (GtkSpinButton *)GTK_WIDGET(gtk_builder_get_object(p_builder, ev_comp.c_str()));
                if (m_p_ev_comp != nullptr) {
                    // set defaults and ranges
                    adjustment = gtk_adjustment_new(0.0, -10.0, 10.0, 0.25, 1.0, 0.0);
                    gtk_spin_button_configure(m_p_ev_comp, adjustment, 1.0, 2);
                 }

                 m_p_sharpness = (GtkSpinButton *)GTK_WIDGET(gtk_builder_get_object(p_builder, sharpness.c_str()));
                 if (m_p_sharpness != nullptr) {
                    adjustment = gtk_adjustment_new(0.0, -100.0, 100.0, 1.0, 5.0, 0.0);
                    gtk_spin_button_configure(m_p_sharpness, adjustment, 1.0, 0);
                 }

                 m_p_analoggain = (GtkSpinButton *)GTK_WIDGET(gtk_builder_get_object(p_builder, analoggain.c_str()));
                 if (m_p_analoggain != nullptr) {
                    adjustment = gtk_adjustment_new(1.0, 1.0, 12.0, 0.25, 1.0, 0.0);
                    gtk_spin_button_configure(m_p_analoggain, adjustment, 1.0, 2);
                 }

                 m_p_ucoloreffect = (GtkSpinButton *)GTK_WIDGET(gtk_builder_get_object(p_builder, ucoloreffect.c_str()));
                 if (m_p_ucoloreffect != nullptr) {
                    adjustment = gtk_adjustment_new(0.0, 0.0, 255.0, 1.0, 5.0, 0.0);
                    gtk_spin_button_configure(m_p_ucoloreffect, adjustment, 1.0, 0);
                 }

                 m_p_vcoloreffect = (GtkSpinButton *)GTK_WIDGET(gtk_builder_get_object(p_builder, vcoloreffect.c_str()));
                 if (m_p_vcoloreffect != nullptr) {
                    adjustment = gtk_adjustment_new(0.0, 0.0, 255.0, 1.0, 5.0, 0.0);
                    gtk_spin_button_configure(m_p_vcoloreffect, adjustment, 1.0, 0);
                 }
            }

            return success;
        }
    }
}