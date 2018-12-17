/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of Swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "statusmessage.h"
#include "statusexception.h"
#include "propertyindex.h"
#include "iconlist.h"
#include "loghandler.h"
#include "logmessage.h"
#include "comparefunctions.h"
#include "stringutils.h"

#include <QMetaEnum>
#include <QStringBuilder>

namespace BlackMisc
{
    namespace Private
    {
        namespace
        {
            template <size_t... Is> QString arg(std::index_sequence<Is...>, const QString &format, const QStringList &args) { return format.arg(args[Is]...); }
            QString arg(std::index_sequence<>, const QString &format, const QStringList &) { return format; }
        }

        QString arg(const QString &format, const QStringList &args)
        {
            if (format.isEmpty())
            {
                return args.join(" ");
            }
            else
            {
                switch (args.size())
                {
                case 0: return arg(std::make_index_sequence<0>(), format, args);
                case 1: return arg(std::make_index_sequence<1>(), format, args);
                case 2: return arg(std::make_index_sequence<2>(), format, args);
                case 3: return arg(std::make_index_sequence<3>(), format, args);
                case 4: return arg(std::make_index_sequence<4>(), format, args);
                case 5: return arg(std::make_index_sequence<5>(), format, args);
                case 6: return arg(std::make_index_sequence<6>(), format, args);
                case 7: return arg(std::make_index_sequence<7>(), format, args);
                case 8: return arg(std::make_index_sequence<8>(), format, args);
                default: qWarning("Too many arguments to BlackMisc::Private::arg"); // intentional fall-through
                case 9: return arg(std::make_index_sequence<9>(), format, args);
                }
            }
        }
    }

    // needed because these constants are odr-used (just like traditional C++98 static const)
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54483
    const StatusSeverity CStatusMessage::SeverityDebug;
    const StatusSeverity CStatusMessage::SeverityInfo;
    const StatusSeverity CStatusMessage::SeverityWarning;
    const StatusSeverity CStatusMessage::SeverityError;

    CStatusMessage::CStatusMessage(const CLogCategory &category) :
        CMessageBase(category), ITimestampBased(QDateTime::currentMSecsSinceEpoch())
    {}

    CStatusMessage::CStatusMessage(const CLogCategoryList &categories) :
        CMessageBase(categories), ITimestampBased(QDateTime::currentMSecsSinceEpoch())
    {}

    CStatusMessage::CStatusMessage(const CLogCategoryList &categories, const CLogCategory &extra) :
        CMessageBase(categories, extra), ITimestampBased(QDateTime::currentMSecsSinceEpoch())
    {}

    CStatusMessage::CStatusMessage(const CLogCategoryList &categories, const CLogCategoryList &extra) :
        CMessageBase(categories, extra), ITimestampBased(QDateTime::currentMSecsSinceEpoch())
    {}

    CStatusMessage::CStatusMessage(): ITimestampBased(QDateTime::currentMSecsSinceEpoch())
    {}

    CStatusMessage::CStatusMessage(const CStatusMessage &other) :
        CValueObject(other),
        CMessageBase(other),
        ITimestampBased(other)
    {
        QReadLocker lock(&other.m_lock);
        m_handledByObjects = other.m_handledByObjects;
    }

    CStatusMessage &CStatusMessage::operator =(const CStatusMessage &other)
    {
        // locks because of mutable members
        if (this == &other) { return *this; }

        static_cast<CMessageBase &>(*this) = other;

        QReadLocker readLock(&other.m_lock);
        auto handledBy = other.m_handledByObjects;
        qint64 ts = other.m_timestampMSecsSinceEpoch;
        readLock.unlock(); // avoid deadlock

        QWriteLocker writeLock(&m_lock);
        m_handledByObjects = handledBy;
        m_timestampMSecsSinceEpoch = ts;
        return *this;
    }

    CStatusMessage::CStatusMessage(const QString &message) : ITimestampBased(QDateTime::currentMSecsSinceEpoch())
    {
        m_message = message.trimmed();
    }

    CStatusMessage::CStatusMessage(StatusSeverity severity, const QString &message)
        : CStatusMessage(message)
    {
        m_severity = severity;
    }

    CStatusMessage::CStatusMessage(const CLogCategoryList &categories, StatusSeverity severity, const QString &message, bool validation)
        : CStatusMessage(severity, message)
    {
        m_categories = categories;
        if (validation)
        {
            this->addValidationCategory();
        }
    }

    CStatusMessage::CStatusMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
        : CStatusMessage(message.trimmed())
    {
        m_categories = CLogCategoryList::fromQString(context.category);

        switch (type)
        {
        default:
        case QtDebugMsg:   m_severity = SeverityDebug; break;
        case QtInfoMsg:    m_severity = SeverityInfo; break;
        case QtWarningMsg: m_severity = SeverityWarning; break;
        case QtCriticalMsg:
        case QtFatalMsg:
            m_severity = SeverityError;
            break;
        }
    }

    void CStatusMessage::toQtLogTriple(QtMsgType *o_type, QString *o_category, QString *o_message) const
    {
        *o_category = m_categories.toQString();
        *o_message = this->getMessage();

        switch (m_severity)
        {
        default:
        case SeverityDebug:   *o_type = QtDebugMsg; break;
        case SeverityInfo:    *o_type = QtInfoMsg; break;
        case SeverityWarning: *o_type = QtWarningMsg; break;
        case SeverityError:   *o_type = QtCriticalMsg; break;
        }
    }

    CStatusException CStatusMessage::asException() const
    {
        return CStatusException(*this);
    }

    void CStatusMessage::maybeThrow() const
    {
        if (! this->isEmpty())
        {
            throw this->asException();
        }
    }

    QString CStatusMessage::getCategoriesAsString() const
    {
        return m_categories.toQString();
    }

    QString CStatusMessage::getHumanReadablePattern() const
    {
        const QStringList patternNames(getHumanReadablePatterns());
        return patternNames.isEmpty() ? QString() : patternNames.join(", ");
    }

    QStringList CStatusMessage::getHumanReadablePatterns() const
    {
        QStringList patternNames;
        for (const QString &name : CLogPattern::allHumanReadableNames())
        {
            if (CLogPattern::fromHumanReadableName(name).match(*this)) { patternNames.push_back(name); }
        }
        return patternNames;
    }

    QString CStatusMessage::getHumanOrTechnicalCategoriesAsString() const
    {
        if (m_categories.isEmpty()) { return ""; }
        const QString c(getHumanReadablePattern());
        return c.isEmpty() ? this->getCategoriesAsString() : c;
    }

    bool CStatusMessage::clampSeverity(CStatusMessage::StatusSeverity severity)
    {
        if (this->getSeverity() <= severity) { return false; }
        this->setSeverity(severity);
        return true;
    }

    bool CStatusMessage::isSeverityHigherOrEqual(CStatusMessage::StatusSeverity severity) const
    {
        return this->getSeverity() >= severity;
    }

    bool CStatusMessage::isSuccess() const
    {
        return !this->isFailure();
    }

    bool CStatusMessage::isFailure() const
    {
        return this->getSeverity() == SeverityError;
    }

    QString CStatusMessage::getMessageNoLineBreaks() const
    {
        const QString m = this->getMessage();
        if (!containsLineBreakOrTab(m)) { return m; } // by far most messages will NOT contain tabs/CR
        return removeLineBreakAndTab(m);
    }

    void CStatusMessage::prependMessage(const QString &msg)
    {
        if (msg.isEmpty()) { return; }
        m_message = msg + m_message;
    }

    void CStatusMessage::appendMessage(const QString &msg)
    {
        if (msg.isEmpty()) { return; }
        m_message += msg;
    }

    void CStatusMessage::markAsHandledBy(const QObject *object) const
    {
        QWriteLocker lock(&m_lock);
        m_handledByObjects.push_back(quintptr(object));
    }

    bool CStatusMessage::wasHandledBy(const QObject *object) const
    {
        QReadLocker lock(&m_lock);
        return m_handledByObjects.contains(quintptr(object));
    }

    QString CStatusMessage::convertToQString(bool /** i18n */) const
    {
        return QLatin1String("Category: ") %
               m_categories.toQString() %

               QLatin1String(" Severity: ") %
               severityToString(m_severity) %

               QLatin1String(" when: ") %
               this->getFormattedUtcTimestampYmdhms() %

               QLatin1String(" ") %
               this->getMessage();
    }

    const CIcon &CStatusMessage::convertToIcon(const CStatusMessage &statusMessage)
    {
        return convertToIcon(statusMessage.getSeverity());
    }

    const CIcon &CStatusMessage::convertToIcon(CStatusMessage::StatusSeverity severity)
    {
        switch (severity)
        {
        case SeverityDebug:   return CIcon::iconByIndex(CIcons::StandardIconUnknown16); // TODO
        case SeverityInfo:    return CIcon::iconByIndex(CIcons::StandardIconInfo16);
        case SeverityWarning: return CIcon::iconByIndex(CIcons::StandardIconWarning16);
        case SeverityError:   return CIcon::iconByIndex(CIcons::StandardIconError16);
        default: return CIcon::iconByIndex(CIcons::StandardIconInfo16);
        }
    }

    const QString &CStatusMessage::convertToIconResource(CStatusMessage::StatusSeverity severity)
    {
        static const QString d;
        static const QString i(":/pastel/icons/pastel/16/infomation.png");
        static const QString w(":/pastel/icons/pastel/16/bullet-error.png");
        static const QString e(":/pastel/icons/pastel/16/close-red.png");

        switch (severity)
        {
        case SeverityDebug:   return d;
        case SeverityInfo:    return i;
        case SeverityWarning: return w;
        case SeverityError:   return e;
        default: return d;
        }
    }

    CStatusMessage CStatusMessage::fromDatabaseJson(const QJsonObject &json)
    {
        QString msgText(json.value("text").toString());
        QString severityText(json.value("severity").toString());
        QString typeText(json.value("type").toString());
        StatusSeverity severity = stringToSeverity(severityText);

        typeText = "swift.db.type." + typeText.toLower().remove(' ');
        CStatusMessage m({ CLogCategory::swiftDbWebservice(), CLogCategory(typeText)}, severity, msgText);
        return m;
    }

    void CStatusMessage::registerMetadata()
    {
        CValueObject<CStatusMessage>::registerMetadata();
        qRegisterMetaType<CStatusMessage::StatusSeverity>();
        qDBusRegisterMetaType<CStatusMessage::StatusSeverity>();
    }

    CStatusMessage::StatusSeverity CStatusMessage::stringToSeverity(const QString &severity)
    {
        // pre-check
        QString severityString(severity.trimmed().toLower());
        if (severityString.isEmpty()) { return SeverityInfo; }

        // hard check
        if (severityString.compare(severityToString(SeverityDebug), Qt::CaseInsensitive) == 0) { return SeverityDebug; }
        if (severityString.compare(severityToString(SeverityInfo), Qt::CaseInsensitive) == 0) { return SeverityInfo; }
        if (severityString.compare(severityToString(SeverityWarning), Qt::CaseInsensitive) == 0) { return SeverityWarning; }
        if (severityString.compare(severityToString(SeverityError), Qt::CaseInsensitive) == 0) { return SeverityError; }

        // not found yet, lenient checks
        QChar s = severityString.at(0);
        if (s == 'd') { return SeverityDebug; }
        if (s == 'i') { return SeverityInfo; }
        if (s == 'w') { return SeverityWarning; }
        if (s == 'e') { return SeverityError; }

        return SeverityInfo;
    }

    const QString &CStatusMessage::severityToString(CStatusMessage::StatusSeverity severity)
    {
        switch (severity)
        {
        case SeverityDebug:   { static const QString d("debug");   return d; }
        case SeverityInfo:    { static const QString i("info");    return i; }
        case SeverityWarning: { static const QString w("warning"); return w; }
        case SeverityError:   { static const QString e("error");   return e; }
        default:
            {
                static const QString x("unknown severity");
                qFatal("Unknown severity");
                return x; // just for compiler warning
            }
        }
    }

    QString CStatusMessage::severitiesToString(const QSet<CStatusMessage::StatusSeverity> &severities)
    {
        auto minmax = std::minmax_element(severities.begin(), severities.end());
        auto min = *minmax.first;
        auto max = *minmax.second;
        if (min == SeverityDebug && max == SeverityError) { static const QString all("all severities"); return all; }
        if (min == SeverityDebug) { QStringLiteral("at or below ") % severityToString(max); }
        if (max == SeverityError) { QStringLiteral("at or above ") % severityToString(min); }
        auto list = severities.toList();
        std::sort(list.begin(), list.end());
        QStringList ret;
        std::transform(list.cbegin(), list.cend(), std::back_inserter(ret), severityToString);
        return ret.join("|");
    }

    const QString &CStatusMessage::getSeverityAsString() const
    {
        return severityToString(m_severity);
    }

    const QStringList &CStatusMessage::allSeverityStrings()
    {
        static const QStringList all { severityToString(SeverityDebug), severityToString(SeverityInfo), severityToString(SeverityWarning), severityToString(SeverityError) };
        return all;
    }

    CVariant CStatusMessage::propertyByIndex(const CPropertyIndex &index) const
    {
        if (index.isMyself()) { return CVariant::from(*this); }
        if (ITimestampBased::canHandleIndex(index)) { return ITimestampBased::propertyByIndex(index); }
        const ColumnIndex i = index.frontCasted<ColumnIndex>();
        switch (i)
        {
        case IndexMessage: return CVariant::from(this->getMessage());
        case IndexSeverity: return CVariant::from(m_severity);
        case IndexSeverityAsString: return CVariant::from(this->getSeverityAsString());
        case IndexCategoriesAsString: return CVariant::from(m_categories.toQString());
        case IndexCategoriesHumanReadableAsString: return CVariant::from(this->getHumanReadablePattern());
        case IndexCategoryHumanReadableOrTechnicalAsString: return CVariant::from(this->getHumanOrTechnicalCategoriesAsString());
        case IndexMessageAsHtml: return CVariant::from(this->toHtml(false, true));
        default: return CValueObject::propertyByIndex(index);
        }
    }

    void CStatusMessage::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
    {
        if (index.isMyself()) { (*this) = variant.to<CStatusMessage>(); return; }
        if (ITimestampBased::canHandleIndex(index)) { ITimestampBased::setPropertyByIndex(index, variant); return; }
        const ColumnIndex i = index.frontCasted<ColumnIndex>();
        switch (i)
        {
        case IndexMessage:
            m_message = variant.value<QString>();
            m_args.clear();
            break;
        case IndexSeverity: m_severity = variant.value<StatusSeverity>(); break;
        case IndexCategoriesAsString: m_categories = variant.value<CLogCategoryList>();  break;
        default: CValueObject::setPropertyByIndex(index, variant); break;
        }
    }

    int CStatusMessage::comparePropertyByIndex(const CPropertyIndex &index, const CStatusMessage &compareValue) const
    {
        if (index.isMyself()) { return Compare::compare(this->getSeverity(), compareValue.getSeverity()); }
        if (ITimestampBased::canHandleIndex(index)) { return ITimestampBased::comparePropertyByIndex(index, compareValue); }
        const ColumnIndex i = index.frontCasted<ColumnIndex>();
        switch (i)
        {
        case IndexMessageAsHtml:
        case IndexMessage:
            return this->getMessage().compare(compareValue.getMessage());
        case IndexSeverity: return Compare::compare(this->getSeverity(), compareValue.getSeverity());
        case IndexSeverityAsString: return this->getSeverityAsString().compare(compareValue.getSeverityAsString());
        case IndexCategoriesAsString: return this->getCategoriesAsString().compare(compareValue.getCategoriesAsString());
        case IndexCategoriesHumanReadableAsString: return this->getHumanReadablePattern().compare(compareValue.getHumanReadablePattern());
        case IndexCategoryHumanReadableOrTechnicalAsString: return this->getHumanOrTechnicalCategoriesAsString().compare(compareValue.getHumanOrTechnicalCategoriesAsString());
        default: break;
        }
        Q_ASSERT_X(false, Q_FUNC_INFO, "Comapre failed");
        return 0;
    }

    QString CStatusMessage::toHtml(bool withIcon, bool withColors) const
    {
        QString img;
        if (withIcon)
        {
            const QString r = convertToIconResource(this->getSeverity());
            if (!r.isEmpty()) { img = QStringLiteral("<img src=\"%1\"> ").arg(r); }
        }

        if (withColors)
        {
            switch (this->getSeverity())
            {
            case SeverityWarning: return img % QStringLiteral("<font color=\"yellow\">") % this->getMessage() % QStringLiteral("</font>");
            case SeverityError:   return img % QStringLiteral("<font color=\"red\">") % this->getMessage() % QStringLiteral("</font>");
            case SeverityDebug: break;
            default: break;
            }
        }
        return img % this->getMessage();
    }
} // ns
