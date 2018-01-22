/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_NETWORK_RAWFSDMESSAGE_H
#define BLACKMISC_NETWORK_RAWFSDMESSAGE_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/metaclass.h"
#include "blackmisc/statusmessagelist.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/variant.h"

#include <QDateTime>
#include <QMetaType>
#include <QString>

namespace BlackMisc
{
    namespace Network
    {
        //! Value object for a raw FSD message
        class BLACKMISC_EXPORT CRawFsdMessage : public CValueObject<CRawFsdMessage>
        {
        public:
            //! Properties by index
            enum ColumnIndex
            {
                IndexReceptionTime = BlackMisc::CPropertyIndex::GlobalIndexCRawFsdMessage,
                IndexRawMessage
            };

            //! Default constructor.
            CRawFsdMessage() {}

            //! Constructor.
            CRawFsdMessage(const QString &rawMessage);

            //! Get raw message
            const QString &getRawMessage() const { return m_rawMessage; }

            //! Set raw message
            void setRawMessage(const QString &rawMessage) { m_rawMessage = rawMessage; }

            //! Get reception time
            QDateTime getReceptionTime() const;

            //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
            CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

            //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
            void setPropertyByIndex(const BlackMisc::CPropertyIndex &index, const CVariant &variant);

            //! \copydoc BlackMisc::Mixin::String::toQString()
            QString convertToQString(bool i18n = false) const;

        private:
            QString m_rawMessage;
            QDateTime m_receptionTime = QDateTime::currentDateTime();

            BLACK_METACLASS(
                CRawFsdMessage,
                BLACK_METAMEMBER(rawMessage),
                BLACK_METAMEMBER(receptionTime)
            );
        };
    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Network::CRawFsdMessage)

#endif // guard
