/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_PLUGINSELECTOR_H
#define BLACKGUI_PLUGINSELECTOR_H

#include "blackgui/blackguiexport.h"

#include <QWidget>
#include <QSignalMapper>

namespace BlackGui
{

    /*!
     * Shows all available plugins in nice list and lets user enable, disable and configure
     * each of them.
     */
    class BLACKGUI_EXPORT CPluginSelector : public QWidget
    {
        Q_OBJECT

    signals:
        //! Emitted when user enables/disables the particular plugin
        void pluginStateChanged(const QString &identifier, bool enabled);

        //! Emitted when user clicks the "Details" button
        void pluginDetailsRequested(const QString &identifier);

    public:
        //! Constructor
        explicit CPluginSelector(QWidget *parent = 0);

        //! Adds the new plugin to the list.
        //! \param identifier Identifier of the plugin.
        //! \param name Name of the plugin
        //! \param enabled Defines whether the plugin is initially enabled or not
        void addPlugin(const QString &identifier, const QString &name, bool enabled = true);

    private slots:
        void ps_handlePluginStateChange();

    private:
        QSignalMapper *m_mapper;

    };

} // namespace BlackGui

#endif // BLACKGUI_PLUGINSELECTOR_H
