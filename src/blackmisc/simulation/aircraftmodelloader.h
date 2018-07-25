/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SIMULATION_IAIRCRAFTMODELLOADER_H
#define BLACKMISC_SIMULATION_IAIRCRAFTMODELLOADER_H

#include "blackmisc/simulation/aircraftmodelinterfaces.h"
#include "blackmisc/simulation/aircraftmodellist.h"
#include "blackmisc/simulation/data/modelcaches.h"
#include "blackmisc/simulation/settings/simulatorsettings.h"
#include "blackmisc/simulation/simulatorinfo.h"
#include "blackmisc/statusmessagelist.h"
#include "blackmisc/blackmiscexport.h"

#include <QDateTime>
#include <QFlags>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>
#include <atomic>
#include <memory>
#include <functional>

namespace BlackMisc
{
    namespace Simulation
    {
        /*!
         * Load the aircraft for a simulator
         * \remark all model loaders share the same model caches of Data::CCentralMultiSimulatorModelCachesProvider
         */
        class BLACKMISC_EXPORT IAircraftModelLoader :
            public Data::CCentralMultiSimulatorModelCachesProvider,
            public IModelsSetable,
            public IModelsUpdatable
        {
            Q_OBJECT
            Q_INTERFACES(BlackMisc::Simulation::IModelsSetable)
            Q_INTERFACES(BlackMisc::Simulation::IModelsUpdatable)
            Q_INTERFACES(BlackMisc::Simulation::IModelsForSimulatorSetable)
            Q_INTERFACES(BlackMisc::Simulation::IModelsForSimulatorUpdatable)

        public:
            //! Log categories
            static const CLogCategoryList &getLogCategories();

            //! Parser mode
            enum LoadModeFlag
            {
                NotSet                = 0,
                LoadDirectly          = 1 << 0,   //!< load synchronously (blocking), normally for testing
                LoadInBackground      = 1 << 1,   //!< load in background, asyncronously
                CacheFirst            = 1 << 2,   //!< always use cache (if it has data)
                CacheSkipped          = 1 << 3,   //!< ignore cache
                CacheOnly             = 1 << 4,   //!< only read cache, never load from disk
                InBackgroundWithCache = LoadInBackground | CacheFirst,   //!< Background, cached
                InBackgroundNoCache   = LoadInBackground | CacheSkipped  //!< Background, not checking cache
            };
            Q_DECLARE_FLAGS(LoadMode, LoadModeFlag)

            //! Load mode
            enum LoadFinishedInfo
            {
                CacheLoaded,   //!< cache was loaded
                ParsedData,    //!< parsed data
                LoadingSkipped //!< Loading skipped (empty directory)
            };

            //! Enum as string
            static const QString &enumToString(LoadFinishedInfo info);

            //! Enum as string
            static const QString &enumToString(LoadModeFlag modeFlag);

            //! Enum as string
            static QString enumToString(LoadMode mode);

            //! Is that mode needing caches synchronized?
            static bool needsCacheSynchronized(LoadMode mode);

            //! Callback to consolidate data, normally with DB data
            //! \remark this has to be a abstarct, as DB handling is subject of BlackCore
            using ModelConsolidationCallback = std::function<int (BlackMisc::Simulation::CAircraftModelList &, bool)>;

            //! Destructor
            virtual ~IAircraftModelLoader();

            //! Loading finished?
            virtual bool isLoadingFinished() const = 0;

            //! Start the loading process from disk.
            //! Optional DB models can be passed and used for data consolidation.
            void startLoading(LoadMode mode = InBackgroundWithCache, const ModelConsolidationCallback &modelConsolidation = {}, const QStringList &modelDirectories = {});

            //! Loading in progress
            //! \threadsafe
            bool isLoadingInProgress() const { return m_loadingInProgress; }

            //! Model directories
            QStringList getModelDirectoriesOrDefault() const;

            //! First directory, can be used when only 1 directory is expected
            QString getFirstModelDirectoryOrDefault() const;

            //! Exclude directories
            QStringList getModelExcludeDirectoryPatterns() const;

            //! Simulator
            const CSimulatorInfo &getSimulator() const { return m_simulator; }

            //! Supported simulator
            bool supportsSimulator(const CSimulatorInfo &simulator) const { return m_simulator == simulator; }

            //! Interface implementations @{
            virtual void setModels(const CAircraftModelList &models) override;
            virtual int updateModels(const CAircraftModelList &models) override;
            //! @}

            //! Create a loader and synchronize caches
            static IAircraftModelLoader *createModelLoader(const CSimulatorInfo &simulator, QObject *parent = nullptr);

        signals:
            //! Disk loading started
            //! \remark will only indicate loading from disk, not cache loading
            void diskLoadingStarted(const CSimulatorInfo &simulator, LoadMode loadMode);

            //! Parsing is finished or cache has been loaded
            //! \remark does to fire if the cache has been changed elsewhere and it has just been reloaded here!
            void loadingFinished(const CStatusMessageList &status, const CSimulatorInfo &simulator, LoadFinishedInfo info);

        protected:
            //! Constructor
            IAircraftModelLoader(const CSimulatorInfo &simulator, QObject *parent = nullptr);

            //! Start the loading process from disk
            virtual void startLoadingFromDisk(LoadMode mode, const ModelConsolidationCallback &modelConsolidation, const QStringList &modelDirectories) = 0;

            //! Get model directories from settings if empty, otherwise checked and UNC path fixed
            QStringList getInitializedModelDirectories(const QStringList &modelDirectories, const CSimulatorInfo &simulator) const;

            //! Any cached data?
            bool hasCachedData() const;

            const CSimulatorInfo m_simulator;                         //!< related simulator
            std::atomic<bool>    m_loadingInProgress { false };       //!< loading in progress
            std::atomic<bool>    m_cancelLoading { false };           //!< flag, requesting to cancel loading
            std::atomic<bool>    m_skipLoadingEmptyModelDir { true }; //!< loading empty model dirs might erase the cache, so normally we skip it
            CStatusMessageList   m_loadingMessages;                   //!< loading messages
            Settings::CMultiSimulatorSettings m_settings { this };    //!< settings

        private:
            //! Descriptive name for loader
            void setObjectInfo(const CSimulatorInfo &simulatorInfo);

            //! Loading completed
            void onLoadingFinished(const CStatusMessageList &statusMsgs, const CSimulatorInfo &simulator, LoadFinishedInfo info);
        };

        /*!
         * Single instances of all model loaders (lazy init)
         */
        class BLACKMISC_EXPORT CMultiAircraftModelLoaderProvider : public QObject
        {
            Q_OBJECT

        public:
            //! Loader instance
            IAircraftModelLoader *loaderInstance(const CSimulatorInfo &simulator);

            //! Singleton
            static CMultiAircraftModelLoaderProvider &multiModelLoaderInstance();

            //! Simulator specific loaders @{
            IAircraftModelLoader *modelLoaderFsx() const { return m_loaderFsx; }
            IAircraftModelLoader *modelLoaderP3D() const { return m_loaderP3D; }
            IAircraftModelLoader *modelLoaderXP()  const { return m_loaderXP; }
            IAircraftModelLoader *modelLoaderFS9() const { return m_loaderFS9; }
            //! @}

        signals:
            //! \copydoc IAircraftModelLoader::loadingFinished
            void loadingFinished(const CStatusMessageList &status, const CSimulatorInfo &simulator, IAircraftModelLoader::LoadFinishedInfo info);

            //! \copydoc IAircraftModelLoader::diskLoadingStarted
            void diskLoadingStarted(const CSimulatorInfo &simulator, IAircraftModelLoader::LoadMode mode);

        private:
            IAircraftModelLoader *m_loaderFsx = nullptr;
            IAircraftModelLoader *m_loaderP3D = nullptr;
            IAircraftModelLoader *m_loaderXP  = nullptr;
            IAircraftModelLoader *m_loaderFS9 = nullptr;

            //! Init the loader
            IAircraftModelLoader *initLoader(const CSimulatorInfo &simulator);
        };
    } // ns
} // ns

Q_DECLARE_METATYPE(BlackMisc::Simulation::IAircraftModelLoader::LoadMode)
Q_DECLARE_METATYPE(BlackMisc::Simulation::IAircraftModelLoader::LoadModeFlag)
Q_DECLARE_METATYPE(BlackMisc::Simulation::IAircraftModelLoader::LoadFinishedInfo)
Q_DECLARE_OPERATORS_FOR_FLAGS(BlackMisc::Simulation::IAircraftModelLoader::LoadMode)

#endif // guard
