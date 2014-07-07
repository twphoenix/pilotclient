/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISC_CLIENTLIST_H
#define BLACKMISC_CLIENTLIST_H

#include "nwclient.h"
#include "sequence.h"
#include "collection.h"
#include <QObject>
#include <QString>
#include <QList>

namespace BlackMisc
{
    namespace Network
    {
        //! Value object encapsulating a list of voice rooms.
        class CClientList : public CSequence<CClient>
        {
        public:
            //! Default constructor.
            CClientList();

            //! Construct from a base class object.
            CClientList(const CSequence &other);

            //! QVariant, required for DBus QVariant lists
            virtual QVariant toQVariant() const override { return QVariant::fromValue(*this); }

            //! Register metadata
            static void registerMetadata();

        };

    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Network::CClientList)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackMisc::Network::CClient>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Network::CClient>)

#endif //guard
