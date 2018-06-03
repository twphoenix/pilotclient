/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "applicationinfoview.h"

using namespace BlackMisc;
using namespace BlackGui::Models;

namespace BlackGui
{
    namespace Views
    {
        CApplicationInfoView::CApplicationInfoView(QWidget *parent) : CViewBase(parent)
        {
            this->standardInit(new CApplicationInfoListModel(this));
        }

        int CApplicationInfoView::otherSwiftVersionsFromDataDirectories()
        {
            const CApplicationInfoList others = CApplicationInfoList::fromOtherSwiftVersionsFromDataDirectories();
            this->updateContainer(others);
            return others.size();
        }
    } // namespace
} // namespace