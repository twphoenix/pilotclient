/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_FILTERS_FILTERDIALOG_H
#define BLACKGUI_FILTERS_FILTERDIALOG_H

#include "blackgui/blackguiexport.h"

#include <QDialog>
#include <QObject>

class QWidget;

namespace BlackGui::Filters
{
    //! Base for filter dialog
    class BLACKGUI_EXPORT CFilterDialog : public QDialog
    {
        Q_OBJECT

    public:
        //! Constructor
        CFilterDialog(QWidget *parent = nullptr);

        //! Destructor
        virtual ~CFilterDialog();

    private:
        //! Stylesheet changed
        void onStyleSheetChanged();
    };

} // namespace

#endif // guard
