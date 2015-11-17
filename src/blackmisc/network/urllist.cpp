/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/network/urllist.h"
#include "blackmisc/network/networkutils.h"
#include "blackmisc/math/mathutils.h"

using namespace BlackMisc::Math;

namespace BlackMisc
{
    namespace Network
    {
        CUrlList::CUrlList() { }

        CUrlList::CUrlList(const QStringList &listOfUrls)
        {
            for (const QString &url : listOfUrls)
            {
                this->push_back(CUrl(url));
            }
        }

        CUrlList::CUrlList(const CSequence<CUrl> &other) :
            CSequence<CUrl>(other)
        { }

        CUrl CUrlList::getRandomUrl() const
        {
            if (this->isEmpty()) { return CUrl(); }
            if (this->size() == 1) { return this->front();}
            int i = CMathUtils::randomInteger(0, this->size() - 1);
            return (*this)[i];
        }

        CUrl CUrlList::getRandomWorkingUrl(int maxTrials) const
        {
            if (this->isEmpty()) { return CUrl(); }
            if (maxTrials < 1) { return CUrl();}
            CUrlList trials;

            for (int t = 0; t < maxTrials && t < this->size(); t++)
            {
                CUrl url(getRandomWithout(trials));
                trials.push_back(url);
                QString message;
                if (CNetworkUtils::canConnect(url, message)) { return url; }
            }
            return CUrl();
        }

        CUrl CUrlList::getRandomWithout(const CUrlList &exclude) const
        {
            CUrlList copy(*this);
            copy.removeIfIn(exclude);
            if (copy.isEmpty()) { return CUrl(); }
            return copy.getRandomUrl();
        }

        CUrl CUrlList::getNextUrl(bool randomStart) const
        {
            if (this->isEmpty()) { return CUrl(); }
            if (this->size() == 1) { return this->front();}
            if (m_currentIndexDistributedLoad < 0)
            {
                // random start point
                m_currentIndexDistributedLoad =  randomStart ?
                                                 CMathUtils::randomInteger(0, this->size() - 1) :
                                                 0;
            }
            else
            {
                m_currentIndexDistributedLoad++;
                if (m_currentIndexDistributedLoad >= this->size())
                {
                    m_currentIndexDistributedLoad = 0;
                }
            }
            return (*this)[m_currentIndexDistributedLoad];
        }

        CUrl CUrlList::getNextUrlWithout(const CUrlList &exclude, bool randomStart) const
        {
            CUrlList copy(*this);
            copy.removeIfIn(exclude);
            if (copy.isEmpty()) { return CUrl(); }
            return copy.getNextUrl(randomStart);
        }

        CUrlList CUrlList::appendPath(const QString &path) const
        {
            if (path.isEmpty() || this->isEmpty()) { return (*this); }
            CUrlList urls;
            for (const CUrl &url : (*this))
            {
                urls.push_back(url.withAppendedPath(path));
            }
            return urls;
        }

        QString CUrlList::convertToQString(const QString &separator, bool i18n) const
        {
            const QStringList sl(toStringList(i18n));
            return sl.join(separator);
        }

        CFailoverUrlList::CFailoverUrlList(int maxTrials) :
            m_maxTrials(maxTrials)
        { }

        CFailoverUrlList::CFailoverUrlList(const QStringList &listOfUrls, int maxTrials) :
            CUrlList(listOfUrls), m_maxTrials(maxTrials)
        { }

        CFailoverUrlList::CFailoverUrlList(const CSequence<CUrl> &other, int maxTrials) :
            CUrlList(other), m_maxTrials(maxTrials)
        { }

        CUrlList CFailoverUrlList::getWithoutFailed() const
        {
            CUrlList urls(*this);
            urls.removeIfIn(m_failedUrls);
            return urls;
        }

        bool CFailoverUrlList::addFailedUrl(const CUrl &failedUrl)
        {
            this->m_failedUrls.push_back(failedUrl);
            return hasMoreUrlsToTry();
        }

        bool CFailoverUrlList::hasMoreUrlsToTry() const
        {
            if (this->isEmpty()) { return false; }
            return (m_failedUrls.size() < this->size() && m_failedUrls.size() < m_maxTrials);
        }

        CUrl CFailoverUrlList::getNextWorkingUrl(const CUrl &failedUrl, bool random)
        {
            if (!failedUrl.isEmpty()) { this->addFailedUrl(failedUrl); }
            if (!hasMoreUrlsToTry()) { return CUrl(); }
            CUrl url(this->getNextUrl(random));
            if (CNetworkUtils::canConnect(url)) { return url; }
            if (addFailedUrl(url)) { return getNextUrl(); }
            return CUrl();
        }
    } // namespace
} // namespace
