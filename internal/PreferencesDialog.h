/**
 * PreferencesDialog.h
 */

#ifndef _H_PREFERENCES_DIALOG
#define _H_PREFERENCES_DIALOG

#include "AfmWindow.h"

namespace afm
{
    namespace graphic
    {
        class PreferencesDialog : public AfmWindow
        {
            public:
                PreferencesDialog();

                virtual bool Initialize(GtkBuilder *p_builder) override;

            private:
                GtkComboBoxText *m_p_awb = nullptr;
                GtkComboBoxText *m_p_dynamic_range = nullptr;
                GtkComboBoxText *m_p_effects = nullptr;
                GtkComboBoxText *m_p_iso = nullptr;
                GtkComboBoxText *m_p_metering = nullptr;
                GtkComboBoxText *m_p_exposure = nullptr;

                GtkSpinButton   *m_p_ev_comp = nullptr;
                GtkSpinButton   *m_p_sharpness = nullptr;
                GtkSpinButton   *m_p_analoggain = nullptr;
                GtkSpinButton   *m_p_ucoloreffect = nullptr;
                GtkSpinButton   *m_p_vcoloreffect = nullptr;
        };
    }
}
#endif