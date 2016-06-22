/* Copyright (C) 2016
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_SETTINGS_NAVIGATOR_H
#define BLACKGUI_SETTINGS_NAVIGATOR_H

#include "blackgui/blackguiexport.h"
#include "blackmisc/settingscache.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/variant.h"

#include <QMap>
#include <QString>
#include <QMetaType>

namespace BlackGui
{
    namespace Settings
    {
        //! Settings for dockwidget
        class BLACKGUI_EXPORT CSettingsNavigator :
            public BlackMisc::CValueObject<CSettingsNavigator>
        {
        public:
            //! Properties by index
            enum ColumnIndex
            {
                IndexMargins = BlackMisc::CPropertyIndex::GlobalIndexCSettingsNavigator,
                IndexFrameless,
                IndexColumns
            };

            //! Default constructor
            CSettingsNavigator();

            //! Destructor.
            ~CSettingsNavigator() {}

            //! Reset to defaults
            void reset();

            //! Set margins
            void setMargins(const QMargins &margins);

            //! Margins
            QMargins getMargins() const;

            //! Frameless?
            bool isFramless() const { return m_frameless; }

            //! Frameless
            void setFrameless(bool frameless) { m_frameless = frameless; }

            //! Number pf columns
            int getColumns() const { return m_columns; }

            //! Set columns
            void setColumns(int columns) { this->m_columns = columns; }

            //! Geometry
            QByteArray getGeometry() const;

            //! Set geometry
            void setGeometry(const QByteArray &ba);

            //! \copydoc BlackMisc::Mixin::String::toQString
            QString convertToQString(bool i18n = false) const;

            //! To string
            QString convertToQString(const QString &separator, bool i18n = false) const;

            //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
            BlackMisc::CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

            //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
            void setPropertyByIndex(const BlackMisc::CPropertyIndex &index, const BlackMisc::CVariant &variant);

        private:
            QString m_margins {"0:0:0:0"}; //!< margins
            QString m_geometry;            //!< geometry as HEX values
            bool    m_frameless = false;   //!< frameless
            int     m_columns = 1;         //!< number of columns

            BLACK_METACLASS(
                CSettingsNavigator,
                BLACK_METAMEMBER(margins),
                BLACK_METAMEMBER(frameless),
                BLACK_METAMEMBER(columns),
                BLACK_METAMEMBER(geometry)
            );
        };

        //! Trait for settings for navigator
        struct SettingsNavigator : public BlackMisc::CSettingTrait<CSettingsNavigator>
        {
            //! Key in data cache
            static const char *key() { return "guinavigator"; }
        };
    } // ns
} // ns

Q_DECLARE_METATYPE(BlackGui::Settings::CSettingsNavigator)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackGui::Settings::CSettingsNavigator>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackGui::Settings::CSettingsNavigator>)

#endif // guard
