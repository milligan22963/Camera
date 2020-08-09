/**
 * AboutDialog.cpp
 */

#include "AboutDialog.h"

namespace afm
{
    namespace graphic
    {
        const std::string about_dialog = "AboutDlg";

        AboutDialog::AboutDialog()
            : AfmWindow()
        {
            SetWindowName(about_dialog);
        }
    }
}