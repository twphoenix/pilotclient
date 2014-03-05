#include "valueobject.h"
#include "valuemap.h"
#include "blackmiscfreefunctions.h"

namespace BlackMisc
{

    /*
     * Stringify
     */
    QString CValueObject::toQString(bool i18n) const
    {
        return this->convertToQString(i18n);
    }

    /*
     * Stringify
     */
    QString CValueObject::toFormattedQString(bool i18n) const
    {
        return this->toQString(i18n);
    }

    /*
     * Stringify
     */
    std::string CValueObject::toStdString(bool i18n) const
    {
        return this->convertToQString(i18n).toStdString();
    }

    /*
     * Streaming
     */
    QString CValueObject::stringForStreaming() const
    {
        // simplest default implementation requires only one method
        return this->convertToQString();
    }

    /*
     * Setter for property by index
     */
    void CValueObject::setPropertyByIndex(const QVariant & /** variant **/, int /** index **/)
    {
        // not all classes have to implement this
        qFatal("Property by index setter not implemented");
    }

    /*
     * By index
     */
    QVariant CValueObject::propertyByIndex(int /** index **/) const
    {
        // not all classes have to implement this
        qFatal("Property by index not implemented");
        return QVariant("propertyByIndex not implemented"); // avoid compiler warning
    }

    /*
     * By index as string
     */
    QString CValueObject::propertyByIndexAsString(int index, bool i18n) const
    {
        // default implementation, requires propertyByIndex
        QVariant qv = this->propertyByIndex(index);
        return BlackMisc::qVariantToString(qv, i18n);
    }

    /*
     * Return backing streamable object (if any)
     */
    const CValueObject *CValueObject::fromQVariant(const QVariant &qv)
    {
        if (!qv.isValid()) return nullptr;
        QVariant::Type t = qv.type();
        uint ut = qv.userType();
        if (t != QVariant::UserType) return nullptr;  // not a user type
        if (ut <= QVariant::UserType) return nullptr; // complex Qt type
        if (qv.canConvert<QDBusArgument>()) return nullptr; // not unstreamed yet

        // this cast cannot be dynamic, so the aboce conditions are crucical
        const CValueObject *vo = static_cast<const CValueObject *>(qv.constData());
        return vo;
    }

    /*
     * Compare
     */
    int compare(const CValueObject &v1, const CValueObject &v2)
    {
        if (v1.isA(v2.getMetaTypeId()))
        {
            return v2.compareImpl(v1) * -1;
        }
        else if (v2.isA(v1.getMetaTypeId()))
        {
            return v1.compareImpl(v2);
        }
        else
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "Attempt to compare between instances of unrelated classes");
            return 0;
        }
    }

    /*!
     * Variant map
     */
    int CValueObject::apply(const BlackMisc::CValueMap &valueMap)
    {
        if (valueMap.isEmpty()) return 0;
        int c = 0;

        QMap<int, QVariant>::const_iterator it;
        const QMap<int, QVariant> &map = valueMap.map();
        for (it = map.begin(); it != map.end(); ++it)
        {
            this->setPropertyByIndex(it.value(), it.key());
        }
        return c;
    }

    /*
     * Compare with value map
     */
    bool operator==(const CValueMap &valueMap, const CValueObject &valueObject)
    {
        if (valueMap.isEmpty()) return valueMap.isWildcard();
        QMap<int, QVariant>::const_iterator it;
        const QMap<int, QVariant> &map = valueMap.map();
        for (it = map.begin(); it != map.end(); ++it)
        {
            // QVariant cannot be compared directly
            QVariant p = valueObject.propertyByIndex(it.key()); // from value object
            QVariant v = it.value(); // from map
            if (!BlackMisc::equalQVariants(p, v)) return false;
        }
        return true;
    }

    /*
     * Compare with value map
     */
    bool operator!=(const CValueMap &valueMap, const CValueObject &valueObject)
    {
        return !(valueMap == valueObject);
    }

    /*
     * Compare with value map
     */
    bool operator==(const CValueObject &valueObject, const CValueMap &valueMap)
    {
        return valueMap == valueObject;
    }

    /*
     * Compare with value map
     */
    bool operator!=(const CValueObject &valueObject, const CValueMap &valueMap)
    {
        return !(valueMap == valueObject);
    }

    /*
     * from DBus
     */
    const QDBusArgument &operator>>(const QDBusArgument &argument, CValueObject &valueObject)
    {
        argument.beginStructure();
        valueObject.unmarshallFromDbus(argument);
        argument.endStructure();
        return argument;
    }

    /*
     * to DBus
     */
    QDBusArgument &operator<<(QDBusArgument &argument, const CValueObject &valueObject)
    {
        argument.beginStructure();
        valueObject.marshallToDbus(argument);
        argument.endStructure();
        return argument;
    }
}
