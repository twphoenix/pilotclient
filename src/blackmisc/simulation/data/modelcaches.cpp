/* Copyright (C) 2016
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/simulation/data/modelcaches.h"
#include "blackmisc/cachesettingsutils.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/verify.h"
#include <QtGlobal>

using namespace BlackMisc;

namespace BlackMisc
{
    namespace Simulation
    {
        namespace Data
        {
            void IMultiSimulatorModelCaches::setModelsForSimulator(const CAircraftModelList &models, const CSimulatorInfo &simulator)
            {
                this->setCachedModels(models, simulator);
            }

            int IMultiSimulatorModelCaches::updateModelsForSimulator(const CAircraftModelList &models, const CSimulatorInfo &simulator)
            {
                if (models.isEmpty()) { return 0; }
                CAircraftModelList allModels(this->getSynchronizedCachedModels(simulator));
                const int c = allModels.replaceOrAddModelsWithString(models, Qt::CaseInsensitive);
                if (c < 1) { return 0; }
                this->setCachedModels(allModels, simulator);
                return allModels.size();
            }

            QString IMultiSimulatorModelCaches::getInfoString() const
            {
                static const QString is("FSX: %1 P3D: %2 FS9: %3 XP: %4");
                return is.arg(this->getCachedModelsCount(CSimulatorInfo::FSX)).arg(this->getCachedModelsCount(CSimulatorInfo::P3D)).arg(this->getCachedModelsCount(CSimulatorInfo::FS9)).arg(this->getCachedModelsCount(CSimulatorInfo::XPLANE));
            }

            QString IMultiSimulatorModelCaches::getInfoStringFsFamily() const
            {
                static const QString is("FSX: %1 P3D: %2 FS9: %3");
                return is.arg(this->getCachedModelsCount(CSimulatorInfo::FSX)).arg(this->getCachedModelsCount(CSimulatorInfo::P3D)).arg(this->getCachedModelsCount(CSimulatorInfo::FS9));
            }

            QString IMultiSimulatorModelCaches::getCacheCountAndTimestamp(const CSimulatorInfo &simulator) const
            {
                static const QString s("%1 models, ts: %2");
                return s.arg(this->getCachedModelsCount(simulator)).arg(this->getCacheTimestamp(simulator).toString("yyyy-MM-dd HH:mm:ss"));
            }

            void IMultiSimulatorModelCaches::gracefulShutdown()
            {
                // void
            }

            void IMultiSimulatorModelCaches::markCacheAsAlreadySynchronized(const CSimulatorInfo &simulator, bool synchronized)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: m_syncFS9 = synchronized; break;
                case CSimulatorInfo::FSX: m_syncFsx = synchronized; break;;
                case CSimulatorInfo::P3D: m_syncP3D = synchronized; break;;
                case CSimulatorInfo::XPLANE: m_syncXPlane = synchronized; break;;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    break;
                }
            }

            void IMultiSimulatorModelCaches::emitCacheChanged(const CSimulatorInfo &simulator)
            {
                emit this->cacheChanged(simulator);
            }

            int IMultiSimulatorModelCaches::getCachedModelsCount(const CSimulatorInfo &simulator) const
            {
                return this->getCachedModels(simulator).size();
            }

            bool IMultiSimulatorModelCaches::hasOtherVersionFile(const CApplicationInfo &info, const CSimulatorInfo &simulator) const
            {
                const QString fn = this->getFilename(simulator);
                return CCacheSettingsUtils::hasOtherVersionCacheFile(info, fn);
            }

            CSimulatorInfo IMultiSimulatorModelCaches::otherVersionSimulatorsWithFile(const CApplicationInfo &info) const
            {
                CSimulatorInfo sim = CSimulatorInfo(CSimulatorInfo::None);
                if (this->hasOtherVersionFile(info, CSimulatorInfo::fsx())) { sim.addSimulator(CSimulatorInfo::fsx()); }
                if (this->hasOtherVersionFile(info, CSimulatorInfo::p3d())) { sim.addSimulator(CSimulatorInfo::p3d()); }
                if (this->hasOtherVersionFile(info, CSimulatorInfo::fs9())) { sim.addSimulator(CSimulatorInfo::fs9()); }
                if (this->hasOtherVersionFile(info, CSimulatorInfo::xplane())) { sim.addSimulator(CSimulatorInfo::xplane()); }
                return sim;
            }

            QStringList IMultiSimulatorModelCaches::getAllFilenames() const
            {
                return QStringList(
                {
                    this->getFilename(CSimulatorInfo::FS9),
                    this->getFilename(CSimulatorInfo::FSX),
                    this->getFilename(CSimulatorInfo::P3D),
                    this->getFilename(CSimulatorInfo::XPLANE)
                });
            }

            CSimulatorInfo IMultiSimulatorModelCaches::getSimulatorForFilename(const QString &filename) const
            {
                if (filename.isEmpty()) { return CSimulatorInfo(); }
                CSimulatorInfo sims;
                const QString compareFileName(QFileInfo(filename).fileName());
                for (const CSimulatorInfo &singleSim : CSimulatorInfo::allSimulatorsSet())
                {
                    const QString singleSimFile(getFilename(singleSim));
                    if (singleSimFile.isEmpty()) continue;
                    const QString singleSimFileName(QFileInfo(singleSimFile).fileName());
                    if (singleSimFileName == compareFileName)
                    {
                        sims.add(singleSim);
                        break;
                    }
                }
                return sims;
            }

            CAircraftModelList IMultiSimulatorModelCaches::getSynchronizedCachedModels(const CSimulatorInfo &simulator)
            {
                BLACK_VERIFY_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "need single simulator");
                this->synchronizeCache(simulator);
                return this->getCachedModels(simulator);
            }

            QDateTime IMultiSimulatorModelCaches::getSynchronizedTimestamp(const CSimulatorInfo &simulator)
            {
                BLACK_VERIFY_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "need single simulator");
                this->synchronizeCache(simulator);
                return this->getCacheTimestamp(simulator);
            }

            CSimulatorInfo IMultiSimulatorModelCaches::simulatorsWithInitializedCache() const
            {
                static const QDateTime outdated = QDateTime::currentDateTimeUtc().addDays(-365 * 5);
                CSimulatorInfo withInitializedCache;
                for (const CSimulatorInfo &simInfo : CSimulatorInfo::allSimulators().asSingleSimulatorSet())
                {
                    const QDateTime ts = this->getCacheTimestamp(simInfo);
                    if (ts.isValid() && ts > outdated)
                    {
                        withInitializedCache.add(simInfo);
                    }
                }
                return withInitializedCache;
            }

            CModelCaches::CModelCaches(bool synchronizeCache, QObject *parent) : IMultiSimulatorModelCaches(parent)
            {
                const CSimulatorInfo sim = BlackMisc::Simulation::CSimulatorInfo::guessDefaultSimulator();
                const QString simStr(sim.toQString(true));
                if (synchronizeCache)
                {
                    this->synchronizeCacheImpl(sim);
                    CLogMessage(this).info("Initialized model caches (%1) for %2") << this->getDescription() << simStr;
                }
                else
                {
                    this->admitCacheImpl(sim);
                    CLogMessage(this).info("Admit model (%1) caches for %2") << this->getDescription() << simStr;
                }
            }

            const CLogCategoryList &CModelCaches::getLogCategories()
            {
                static const CLogCategoryList l({ CLogCategory::modelCache() });
                return l;
            }

            CAircraftModelList CModelCaches::getCachedModels(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.get();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.get();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.get();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.get();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    return CAircraftModelList();
                }
            }

            CStatusMessage CModelCaches::setCachedModels(const CAircraftModelList &models, const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                CStatusMessage msg;
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: msg = m_modelCacheFs9.set(models); break;
                case CSimulatorInfo::FSX: msg = m_modelCacheFsx.set(models); break;
                case CSimulatorInfo::P3D: msg = m_modelCacheP3D.set(models); break;
                case CSimulatorInfo::XPLANE: msg = m_modelCacheXP.set(models); break;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    return CStatusMessage();
                }
                this->emitCacheChanged(simulator); // set
                return msg;
            }

            CStatusMessage IMultiSimulatorModelCaches::clearCachedModels(const CSimulatorInfo &simulator)
            {
                static const CAircraftModelList models;
                return this->setCachedModels(models, simulator);
            }

            bool IMultiSimulatorModelCaches::isCacheAlreadySynchronized(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_syncFS9;
                case CSimulatorInfo::FSX: return m_syncFsx;
                case CSimulatorInfo::P3D: return m_syncP3D;
                case CSimulatorInfo::XPLANE: return m_syncXPlane;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    break;
                }
                return false;
            }

            void IMultiSimulatorModelCaches::synchronizeMultiCaches(const CSimulatorInfo &simulator)
            {
                for (const CSimulatorInfo &singleSimulator : simulator.asSingleSimulatorSet())
                {
                    this->synchronizeCache(singleSimulator);
                }
            }

            void IMultiSimulatorModelCaches::admitMultiCaches(const CSimulatorInfo &simulator)
            {
                for (const CSimulatorInfo &singleSimulator : simulator.asSingleSimulatorSet())
                {
                    this->admitMultiCaches(singleSimulator);
                }
            }

            QDateTime CModelCaches::getCacheTimestamp(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.getAvailableTimestamp();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.getAvailableTimestamp();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.getAvailableTimestamp();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.getAvailableTimestamp();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    return QDateTime();
                }
            }

            CStatusMessage CModelCaches::setCacheTimestamp(const QDateTime &ts, const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                if (!ts.isValid()) { return CStatusMessage(this).error("Invalid timestamp for '%1'") << simulator.toQString() ; }
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.set(m_modelCacheFs9.get(), ts.toMSecsSinceEpoch());
                case CSimulatorInfo::FSX: return m_modelCacheFsx.set(m_modelCacheFsx.get(), ts.toMSecsSinceEpoch());
                case CSimulatorInfo::P3D: return m_modelCacheP3D.set(m_modelCacheP3D.get(), ts.toMSecsSinceEpoch());
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.set(m_modelCacheXP.get(), ts.toMSecsSinceEpoch());
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Wrong simulator");
                    break;
                }
                return CStatusMessage();
            }

            void CModelCaches::synchronizeCache(const CSimulatorInfo &simulator)
            {
                this->synchronizeCacheImpl(simulator);
            }

            void CModelCaches::admitCache(const CSimulatorInfo &simulator)
            {
                this->admitCacheImpl(simulator);
            }

            QString CModelCaches::getFilename(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.getFilename();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.getFilename();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.getFilename();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.getFilename();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    break;
                }
                return {};
            }

            bool CModelCaches::isSaved(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.isSaved();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.isSaved();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.isSaved();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.isSaved();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    break;
                }
                return false;
            }

            void CModelCaches::synchronizeCacheImpl(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");

                if (this->isCacheAlreadySynchronized(simulator)) { return; }
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: m_modelCacheFs9.synchronize(); break;
                case CSimulatorInfo::FSX: m_modelCacheFsx.synchronize(); break;
                case CSimulatorInfo::P3D: m_modelCacheP3D.synchronize(); break;
                case CSimulatorInfo::XPLANE: m_modelCacheXP.synchronize(); break;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    break;
                }
                this->markCacheAsAlreadySynchronized(simulator, true);
                this->emitCacheChanged(simulator); // sync
            }

            void CModelCaches::admitCacheImpl(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");

                if (this->isCacheAlreadySynchronized(simulator)) { return; }
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: m_modelCacheFs9.admit(); break;
                case CSimulatorInfo::FSX: m_modelCacheFsx.admit(); break;
                case CSimulatorInfo::P3D: m_modelCacheP3D.admit(); break;
                case CSimulatorInfo::XPLANE: m_modelCacheXP.admit(); break;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    break;
                }
            }

            CModelSetCaches::CModelSetCaches(bool synchronizeCache, QObject *parent) : IMultiSimulatorModelCaches(parent)
            {
                CSimulatorInfo simulator = CSimulatorInfo::guessDefaultSimulator();
                const QString simStr(simulator.toQString(true));

                if (synchronizeCache)
                {
                    this->synchronizeCacheImpl(simulator);
                    CLogMessage(this).info("Initialized model caches (%1) for %2") << this->getDescription() << simStr;
                }
                else
                {
                    this->admitCacheImpl(simulator);
                    CLogMessage(this).info("Admit model (%1) caches for %2") << this->getDescription() << simStr;
                }
            }

            const CLogCategoryList &CModelSetCaches::getLogCategories()
            {
                static const CLogCategoryList l({ CLogCategory::modelSetCache() });
                return l;
            }

            CAircraftModelList CModelSetCaches::getCachedModels(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.get();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.get();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.get();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.get();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    return CAircraftModelList();
                }
            }

            CStatusMessage CModelSetCaches::setCachedModels(const CAircraftModelList &models, const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                CAircraftModelList orderedModels(models);
                if (orderedModels.needsOrder())
                {
                    orderedModels.resetOrder();
                }
                else
                {
                    orderedModels.sortAscendingByOrder();
                }

                CStatusMessage msg;
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: msg = m_modelCacheFs9.set(orderedModels); break;
                case CSimulatorInfo::FSX: msg = m_modelCacheFsx.set(orderedModels); break;
                case CSimulatorInfo::P3D: msg = m_modelCacheP3D.set(orderedModels); break;
                case CSimulatorInfo::XPLANE: msg = m_modelCacheXP.set(orderedModels); break;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "wrong simulator");
                    return CStatusMessage();
                }
                this->emitCacheChanged(simulator); // set
                return msg;
            }

            QDateTime CModelSetCaches::getCacheTimestamp(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.getAvailableTimestamp();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.getAvailableTimestamp();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.getAvailableTimestamp();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.getAvailableTimestamp();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Wrong simulator");
                    return QDateTime();
                }
            }

            CStatusMessage CModelSetCaches::setCacheTimestamp(const QDateTime &ts, const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                if (!ts.isValid()) { return CStatusMessage(this).error("Invalid timestamp for '%1'") << simulator.toQString() ; }
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.set(m_modelCacheFs9.get(), ts.toMSecsSinceEpoch());
                case CSimulatorInfo::FSX: return m_modelCacheFsx.set(m_modelCacheFsx.get(), ts.toMSecsSinceEpoch());
                case CSimulatorInfo::P3D: return m_modelCacheP3D.set(m_modelCacheP3D.get(), ts.toMSecsSinceEpoch());
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.set(m_modelCacheXP.get(), ts.toMSecsSinceEpoch());
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Wrong simulator");
                    break;
                }
                return CStatusMessage();
            }

            void CModelSetCaches::synchronizeCache(const CSimulatorInfo &simulator)
            {
                this->synchronizeCacheImpl(simulator);
            }

            void CModelSetCaches::admitCache(const CSimulatorInfo &simulator)
            {
                this->admitCacheImpl(simulator);
            }

            QString CModelSetCaches::getFilename(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.getFilename();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.getFilename();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.getFilename();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.getFilename();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Wrong simulator");
                    break;
                }
                return {};
            }

            bool CModelSetCaches::isSaved(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_modelCacheFs9.isSaved();
                case CSimulatorInfo::FSX: return m_modelCacheFsx.isSaved();
                case CSimulatorInfo::P3D: return m_modelCacheP3D.isSaved();
                case CSimulatorInfo::XPLANE: return m_modelCacheXP.isSaved();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Wrong simulator");
                    break;
                }
                return false;
            }

            void CModelSetCaches::synchronizeCacheImpl(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");

                if (this->isCacheAlreadySynchronized(simulator)) { return; }
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: m_modelCacheFs9.synchronize(); break;
                case CSimulatorInfo::FSX: m_modelCacheFsx.synchronize(); break;
                case CSimulatorInfo::P3D: m_modelCacheP3D.synchronize(); break;
                case CSimulatorInfo::XPLANE: m_modelCacheXP.synchronize(); break;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Wrong simulator");
                    break;
                }
                this->markCacheAsAlreadySynchronized(simulator, true);
                this->emitCacheChanged(simulator); // sync
            }

            void CModelSetCaches::admitCacheImpl(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");

                if (this->isCacheAlreadySynchronized(simulator)) { return; }
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: m_modelCacheFs9.admit(); break;
                case CSimulatorInfo::FSX: m_modelCacheFsx.admit(); break;
                case CSimulatorInfo::P3D: m_modelCacheP3D.admit(); break;
                case CSimulatorInfo::XPLANE: m_modelCacheXP.admit(); break;
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Wrong simulator");
                    break;
                }
            }
        } // ns
    } // ns
} // ns
