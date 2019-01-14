/* Copyright (C) 2019
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_AVIATION_AIRCRAFTCATEGORY_H
#define BLACKMISC_AVIATION_AIRCRAFTCATEGORY_H

#include "blackmisc/db/datastore.h"
#include "blackmisc/blackmiscexport.h"
#include "blackmisc/metaclass.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/statusmessagelist.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/variant.h"

#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <QStringList>

namespace BlackMisc
{
    namespace Aviation
    {
        //! Value object for aircraft categories
        class BLACKMISC_EXPORT CAircraftCategory :
            public CValueObject<CAircraftCategory>,
            public Db::IDatastoreObjectWithIntegerKey
        {
        public:
            //! Properties by index
            enum ColumnIndex
            {
                IndexName = BlackMisc::CPropertyIndex::GlobalIndexCAircraftCategory,
                IndexDescription,
                IndexLevelString,
                IndexPath,
                IndexAssignable
            };

            //! Default constructor.
            CAircraftCategory() {}

            //! Constructor.
            CAircraftCategory(const QString &name, const QString &description, const QString &path, bool assignable);

            //! Get name
            const QString &getName() const { return m_name; }

            //! Set name
            void setName(const QString &name) { m_name = name.trimmed(); }

            //! Designator and DB key
            QString getNameDbKey() const;

            //! Aircraft designator?
            bool hasName() const;

            //! Matching name?
            bool matchesName(const QString &name, Qt::CaseSensitivity cs) const;

            //! Description
            const QString &getDescription() const { return m_description; }

            //! Set description
            void setDescription(const QString &description) { m_description = description.trimmed(); }

            //! Path
            const QString &getPath() const { return m_path; }

            //! Level
            void setLevel(int l1, int l2, int l3);

            //! Level string
            QString getLevelString() const;

            //! Matching path?
            bool matchesPath(const QString &path, Qt::CaseSensitivity cs);

            //! Assignable?
            bool isAssignable() const { return m_assignable; }

            //! Mark/set assignable
            void setAssignable(bool assignable) { m_assignable = assignable; }

            //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
            CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

            //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
            void setPropertyByIndex(const BlackMisc::CPropertyIndex &index, const CVariant &variant);

            //! \copydoc BlackMisc::Mixin::Index::comparePropertyByIndex
            int comparePropertyByIndex(const CPropertyIndex &index, const CAircraftCategory &compareValue) const;

            //! \copydoc BlackMisc::Mixin::String::toQString
            QString convertToQString(bool i18n = false) const;

            //! Validate data
            BlackMisc::CStatusMessageList validate() const;

            //! Null ICAO?
            bool isNull() const;

            //! NULL object
            static const CAircraftCategory &null();

            //! From our database JSON format
            static CAircraftCategory fromDatabaseJson(const QJsonObject &json, const QString &prefix = QString());

        private:
            QString m_name;           //!< name
            QString m_description;    //!< description
            QString m_path;           //!< path
            bool m_assignable = true; //!< can assign to category?
            int m_level[3] = { 0, 0, 0};
            BLACK_METACLASS(
                CAircraftCategory,
                BLACK_METAMEMBER(dbKey),
                BLACK_METAMEMBER(timestampMSecsSinceEpoch),
                BLACK_METAMEMBER(name),
                // BLACK_METAMEMBER(level),
                BLACK_METAMEMBER(description),
                BLACK_METAMEMBER(path),
                BLACK_METAMEMBER(assignable)
            );
        };
    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Aviation::CAircraftCategory)

#endif // guard
