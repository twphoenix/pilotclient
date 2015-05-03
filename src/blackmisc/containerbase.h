/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_CONTAINERBASE_H
#define BLACKMISC_CONTAINERBASE_H

#include "range.h"
#include "propertyindexvariantmap.h"
#include "blackmiscfreefunctions.h"
#include "predicates.h"
#include "json.h"
#include <algorithm>

namespace BlackMisc
{

    //! Class providing static helper methods for different containers
    class CContainerHelper
    {
    public:
        //! Stringify value object
        template <class U> static QString stringify(const U &obj, bool i18n) { return obj.toQString(i18n); }
        //! Stringify int
        static QString stringify(int n, bool i18n) { return i18n ? QLocale().toString(n) : QString::number(n); }
        //! Stringify uint
        static QString stringify(uint n, bool i18n) { return i18n ? QLocale().toString(n) : QString::number(n); }
        //! Stringify qlonglong
        static QString stringify(qlonglong n, bool i18n) { return i18n ? QLocale().toString(n) : QString::number(n); }
        //! Stringify qulonglong
        static QString stringify(qulonglong n, bool i18n) { return i18n ? QLocale().toString(n) : QString::number(n); }
        //! Stringify double
        static QString stringify(double n, bool i18n) { return i18n ? QLocale().toString(n) : QString::number(n); }
        //! Stringify QString
        static QString stringify(QString str, bool /*i18n*/) { return str; }
    };

    // forward declaration
    template <template <class> class C, class T, class CIt>
    class CContainerBase;

    //! \private
    template <template <class> class C, class T, class CIt>
    struct CValueObjectPolicy<CContainerBase<C, T, CIt>> : public CValueObjectLegacy
    {
        using MetaType = Policy::MetaType::None;
    };

    /*!
     * \brief Base class for CCollection and CSequence adding mutating operations and CValueObject facility on top of CRangeBase.
     */
    template <template <class> class C, class T, class CIt>
    class CContainerBase : public CValueObject<CContainerBase<C, T, CIt>>, public CRangeBase<C<T>, CIt>
    {
    public:

        //! \copydoc BlackMisc::CValueObject::compare
        friend int compare(const C<T> &a, const C<T> &b)
        {
            if (a.size() < b.size()) { return -1; }
            if (a.size() > b.size()) { return 1; }
            return std::lexicographical_compare(a.cbegin(), a.cend(), b.cbegin(), b.cend());
        }

        /*!
         * \brief Return a new container of a different type, containing the same elements as this one.
         * \tparam Other the type of the new container.
         * \param other an optional initial value for the new container; will be copied.
         */
        template <template <class> class Other>
        Other<T> to(Other<T> other = Other<T>()) const
        {
            for (auto it = derived().cbegin(); it != derived().cend(); ++it) { other.push_back(*it); }
            return other;
        }

        /*!
         * \brief Remove elements for which a given predicate returns true.
         * \pre The sequence must be initialized.
         * \return The number of elements removed.
         */
        template <class Predicate>
        int removeIf(Predicate p)
        {
            int count = 0;
            for (auto it = derived().begin(); it != derived().end();)
            {
                if (p(*it)) { it = derived().erase(it); count++; }
                else { ++it; }
            }
            return count;
        }

        /*!
         * \brief Remove elements matching some particular key/value pair(s).
         * \param k0 A pointer to a member function of T.
         * \param v0 A value to compare against the value returned by k0.
         * \param keysValues Zero or more additional pairs of { pointer to member function of T, value to compare it against }.
         * \return The number of elements removed.
         */
        template <class K0, class V0, class... KeysValues>
        int removeIf(K0 k0, V0 v0, KeysValues... keysValues)
        {
            return removeIf(BlackMisc::Predicates::MemberEqual(k0, v0, keysValues...));
        }

    public:
        //! \copydoc BlackMisc::CValueObject::toQVariant
        virtual QVariant toQVariant() const override { return QVariant::fromValue(derived()); }

        //! \copydoc CValueObject::convertFromQVariant
        virtual void convertFromQVariant(const QVariant &variant) override { BlackMisc::setFromQVariant< C<T> >(&derived(), variant); }

        //! Simplifies composition, returns 0 for performance
        friend uint qHash(const C<T> &) { return 0; }

        //! \copydoc CValueObject::toJson
        QJsonObject toJson() const
        {
            QJsonArray array;
            QJsonObject json;
            for (auto it = derived().cbegin(); it != derived().cend(); ++it)
            {
                array << (*it);
            }
            json.insert("containerbase", array);
            return json;
        }

        //! \copydoc CValueObject::convertFromJson
        void convertFromJson(const QJsonObject &json)
        {
            QJsonArray array = json.value("containerbase").toArray();
            for (auto i = array.begin(); i != array.end(); ++i)
            {
                QJsonValueRef ref = (*i);
                T value;
                ref >> value;
                derived().insert(value);
            }
        }

    protected: // CValueObject overrides
        //! \copydoc BlackMisc::CValueObject::convertToQString
        virtual QString convertToQString(bool i18n = false) const override
        {
            QString str;
            for (const auto &value : derived()) { str += (str.isEmpty() ? "{" : ", ") + CContainerHelper::stringify(value, i18n); }
            if (str.isEmpty()) { str = "{"; }
            return str += "}";
        }

        //! \copydoc BlackMisc::CValueObject::getMetaTypeId
        int getMetaTypeId() const { return qMetaTypeId<C<T>>(); }

    public:
        //! \copydoc BlackMisc::CValueObject::marshallToDbus
        void marshallToDbus(QDBusArgument &argument) const
        {
            argument.beginArray(qMetaTypeId<T>());
            std::for_each(derived().cbegin(), derived().cend(), [ & ](const T & value) { argument << value; });
            argument.endArray();
        }

        //! \copydoc BlackMisc::CValueObject::unmarshallFromDbus
        void unmarshallFromDbus(const QDBusArgument &argument)
        {
            argument.beginArray();
            while (!argument.atEnd()) { T value; argument >> value; derived().insert(value); }
            argument.endArray();
        }

    private:
        C<T> &derived() { return static_cast<C<T> &>(*this); }
        const C<T> &derived() const { return static_cast<const C<T> &>(*this); }
    };

}

#endif // guard
