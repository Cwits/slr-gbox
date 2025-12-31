// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/display/DialogWindows.h"

namespace UI {

InfoDialog::InfoDialog(BaseWidget * parent, UIContext * const uictx) : Popup(parent, uictx) {

}

InfoDialog::~InfoDialog() {

}

DecisionDialog::DecisionDialog(BaseWidget * parent, UIContext * const uictx) : Popup(parent, uictx) {

}

DecisionDialog::~DecisionDialog() {

}


}