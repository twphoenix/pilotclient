/* Copyright (C) 2017
 * swift project Community/Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "downloadcomponent.h"
#include "ui_downloadcomponent.h"
#include "blackgui/guiapplication.h"
#include "blackgui/overlaymessagesframe.h"
#include "blackmisc/simulation/xplane/xplaneutil.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/directoryutils.h"
#include "blackmisc/fileutils.h"

#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>
#include <QDesktopServices>

using namespace BlackMisc;
using namespace BlackMisc::Db;
using namespace BlackMisc::Network;
using namespace BlackMisc::Simulation;

namespace BlackGui
{
    namespace Components
    {
        CDownloadComponent::CDownloadComponent(QWidget *parent) :
            COverlayMessagesFrame(parent),
            CLoadIndicatorEnabled(this),
            ui(new Ui::CDownloadComponent)
        {
            ui->setupUi(this);
            this->setOverlaySizeFactors(0.8, 0.9, 2);
            this->setForceSmall(true);

            ui->le_DownloadDir->setText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            ui->prb_Current->setMinimum(0);
            ui->prb_Current->setMaximum(1); // min/max 0,0 means busy indicator
            ui->prb_Current->setValue(0);
            ui->prb_Total->setMinimum(0);
            ui->prb_Total->setMaximum(1);
            ui->prb_Total->setValue(0);

            connect(ui->tb_DialogDownloadDir, &QToolButton::pressed, this, &CDownloadComponent::selectDownloadDirectory);
            connect(ui->tb_ResetDownloadDir, &QToolButton::pressed, this, &CDownloadComponent::resetDownloadDir);
            connect(ui->tb_CancelDownload, &QToolButton::pressed, this, &CDownloadComponent::cancelOngoingDownloads);
            connect(ui->pb_Download, &QPushButton::pressed, [ = ] { this->triggerDownloadingOfFiles(); });
            connect(ui->pb_OpenDownloadDir, &QPushButton::pressed, this, &CDownloadComponent::openDownloadDir);
            connect(ui->pb_DoIt, &QPushButton::pressed, this, &CDownloadComponent::startDownloadedExecutable);
        }

        CDownloadComponent::~CDownloadComponent()
        { }

        bool CDownloadComponent::setDownloadFile(const CRemoteFile &remoteFile)
        {
            return this->setDownloadFiles(CRemoteFileList(remoteFile));
        }

        bool CDownloadComponent::setDownloadFiles(const CRemoteFileList &remoteFiles)
        {
            if (!m_waitingForDownload.isEmpty()) { return false; }
            m_remoteFiles = remoteFiles;
            this->clear();
            return true;
        }

        bool CDownloadComponent::setDownloadDirectory(const QString &path)
        {
            const QDir d(path);
            if (!d.exists()) return false;
            ui->le_DownloadDir->setText(d.absolutePath());
            return true;
        }

        void CDownloadComponent::selectDownloadDirectory()
        {
            QString downloadDir = ui->le_DownloadDir->text().trimmed();
            downloadDir = QFileDialog::getExistingDirectory(parentWidget(),
                          tr("Choose your download directory"), downloadDir, m_fileDialogOptions);

            if (downloadDir.isEmpty()) { return; } // canceled
            if (!QDir(downloadDir).exists())
            {
                const CStatusMessage msg = CStatusMessage(this, CLogCategory::validation()).warning("'%1' is not a valid download directory") << downloadDir;
                this->showOverlayMessage(msg, CDownloadComponent::OverlayMsgTimeoutMs);
                return;
            }
            ui->le_DownloadDir->setText(downloadDir);
        }

        bool CDownloadComponent::triggerDownloadingOfFiles(int delayMs)
        {
            if (m_remoteFiles.isEmpty()) { return false; }
            if (!m_waitingForDownload.isEmpty()) { return false; }
            if (delayMs > 0)
            {
                QTimer::singleShot(delayMs, this, [ = ] { this->triggerDownloadingOfFiles(); });
                return true;
            }
            m_waitingForDownload = m_remoteFiles;
            this->showFileInfo();
            return this->triggerDownloadingOfNextFile();
        }

        bool CDownloadComponent::isDownloading() const
        {
            return m_reply || m_fileInProgress.hasName();
        }

        bool CDownloadComponent::haveAllDownloadsCompleted() const
        {
            if (this->isDownloading()) { return false; }
            if (!m_waitingForDownload.isEmpty()) { return false; }
            return true;
        }

        CDownloadComponent::Mode CDownloadComponent::getMode() const
        {
            Mode mode = ui->cb_Shutdown->isChecked() ? ShutdownSwift : JustDownload;
            if (ui->cb_StartAfterDownload) mode |= StartAfterDownload;
            return mode;
        }

        void CDownloadComponent::setMode(Mode mode)
        {
            ui->cb_Shutdown->setChecked(mode.testFlag(ShutdownSwift));
            ui->cb_StartAfterDownload->setChecked(mode.testFlag(StartAfterDownload));
        }

        void CDownloadComponent::clear()
        {
            if (m_reply)
            {
                m_reply->abort();
                m_reply = nullptr;
            }

            m_waitingForDownload.clear();
            m_fileInProgress = CRemoteFile();

            ui->prb_Current->setValue(0);
            ui->prb_Total->setValue(0);

            ui->le_Completed->clear();
            ui->le_CompletedNumber->clear();
            ui->le_CompletedUrl->clear();
            ui->le_Started->clear();
            ui->le_StartedNumber->clear();
            ui->le_StartedUrl->clear();
            this->showFileInfo();
        }

        bool CDownloadComponent::triggerDownloadingOfNextFile()
        {
            if (m_waitingForDownload.isEmpty()) { return false; }
            const CRemoteFile rf = m_waitingForDownload.front();
            m_waitingForDownload.pop_front();
            return this->triggerDownloadingOfFile(rf);
        }

        bool CDownloadComponent::triggerDownloadingOfFile(const CRemoteFile &remoteFile)
        {
            if (!sGui || !sGui->hasWebDataServices() || sGui->isShuttingDown()) { return false; }
            if (!this->existsDownloadDir())
            {
                const CStatusMessage msg = CStatusMessage(this, CLogCategory::validation()).error("Invalid download directory");
                this->showOverlayMessage(msg, CDownloadComponent::OverlayMsgTimeoutMs);
                return false;
            }

            const CUrl download = remoteFile.getSmartUrl();
            if (download.isEmpty())
            {
                const CStatusMessage msg = CStatusMessage(this, CLogCategory::validation()).error("No download URL for file name '%1'") << remoteFile.getNameAndSize();
                this->showOverlayMessage(msg, CDownloadComponent::OverlayMsgTimeoutMs);
                return false;
            }

            this->showStartedFileMessage(remoteFile);
            m_fileInProgress = remoteFile;
            const QString saveAsFile = CFileUtils::appendFilePaths(ui->le_DownloadDir->text(), remoteFile.getName());
            QNetworkReply *r = sGui->downloadFromNetwork(download, saveAsFile, { this, &CDownloadComponent::downloadedFile});
            bool success = false;
            if (r)
            {
                // this->showLoading(10 * 1000);
                CLogMessage(this).info("Triggered downloading of file from '%1'") << download.getHost();
                connect(r, &QNetworkReply::downloadProgress, this, &CDownloadComponent::downloadProgress);
                m_reply = r;
                success = true;
            }
            else
            {
                const CStatusMessage msg = CStatusMessage(this, CLogCategory::validation()).error("Starting download for '%1' failed") << download.getFullUrl();
                this->showOverlayMessage(msg, CDownloadComponent::OverlayMsgTimeoutMs);
            }
            return success;
        }

        void CDownloadComponent::downloadedFile(const CStatusMessage &status)
        {
            // reset in progress
            const CRemoteFile justDownloaded(m_fileInProgress);
            m_fileInProgress = CRemoteFile();
            m_reply = nullptr;
            this->showCompletedFileMessage(justDownloaded);
            this->hideLoading();

            if (sGui && sGui->isShuttingDown()) { return; }
            if (status.isWarningOrAbove())
            {
                this->showOverlayMessage(status, CDownloadComponent::OverlayMsgTimeoutMs);
                this->clear();
                return;
            }

            const bool t = this->triggerDownloadingOfNextFile();
            if (!t) { this->lastFileDownloaded(); }
        }

        void CDownloadComponent::lastFileDownloaded()
        {
            QTimer::singleShot(0, this, &CDownloadComponent::allDownloadsCompleted);
            this->startDownloadedExecutable();
        }

        void CDownloadComponent::startDownloadedExecutable()
        {
            if (!ui->cb_StartAfterDownload->isChecked()) { return; }
            if (!this->haveAllDownloadsCompleted()) { return; }
            const CRemoteFileList executables = m_remoteFiles.findExecutableFiles();
            if (executables.isEmpty()) { return; }

            // try to start
            const QDir dir(ui->le_DownloadDir->text());
            if (!dir.exists()) { return; }

            const QString msg = ui->cb_Shutdown->isChecked() ?
                                QString("Start '%1' and close swift?") :
                                QString("Start '%1'?");

            for (const CRemoteFile &rf : executables)
            {
                const QString executable = CFileUtils::appendFilePaths(dir.absolutePath(), rf.getName());
                const QFile executableFile(executable);
                if (!executableFile.exists()) { continue; }

                QMessageBox::StandardButton reply = QMessageBox::question(this, "Start?", msg.arg(rf.getName()), QMessageBox::Yes | QMessageBox::No);
                if (reply != QMessageBox::Yes) { return; }

                QStringList arguments;
                if (rf.isSwiftInstaller())
                {
                    QDir dir(QCoreApplication::applicationDirPath());
                    dir.cdUp();
                    if (dir.exists())
                    {
                        const QString d(dir.absolutePath());
                        arguments << "--installdir";
                        arguments << d;
                    }
                }

                const bool started = QProcess::startDetached(executable, arguments, dir.absolutePath());
                if (started)
                {
                    const bool shutdown = ui->cb_Shutdown->isChecked();
                    if (sGui && shutdown)
                    {
                        QTimer::singleShot(1000, sGui, [] { CGuiApplication::exit(); });
                    }
                    break;
                }
            } // files
        }

        bool CDownloadComponent::existsDownloadDir() const
        {
            if (ui->le_DownloadDir->text().isEmpty()) { return false; }
            const QDir dir(ui->le_DownloadDir->text());
            return dir.exists() && dir.isReadable();
        }

        void CDownloadComponent::openDownloadDir()
        {
            if (!this->existsDownloadDir()) { return; }
            QDesktopServices::openUrl(QUrl::fromLocalFile(ui->le_DownloadDir->text()));
        }

        void CDownloadComponent::resetDownloadDir()
        {
            ui->le_DownloadDir->setText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        }

        void CDownloadComponent::showStartedFileMessage(const CRemoteFile &rf)
        {
            static const QString of("%1/%2");
            const int current = m_remoteFiles.size() - m_waitingForDownload.size();
            ui->le_Started->setText(rf.getName());
            ui->le_StartedNumber->setText(of.arg(current).arg(m_remoteFiles.size()));
            ui->le_StartedUrl->setText(rf.getUrl().getFullUrl());
            ui->prb_Total->setMaximum(m_remoteFiles.size());
            ui->prb_Total->setValue(current - 1);
        }

        void CDownloadComponent::showCompletedFileMessage(const CRemoteFile &rf)
        {
            static const QString of("%1/%2");
            const int current = m_remoteFiles.size() - m_waitingForDownload.size();
            ui->le_Completed->setText(rf.getName());
            ui->le_CompletedNumber->setText(of.arg(current).arg(m_remoteFiles.size()));
            ui->le_CompletedUrl->setText(rf.getUrl().getFullUrl());
            ui->prb_Total->setMaximum(m_remoteFiles.size());
            ui->prb_Total->setValue(current);
        }

        void CDownloadComponent::cancelOngoingDownloads()
        {
            this->clear();
        }

        void CDownloadComponent::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
        {
            ui->prb_Current->setMaximum(bytesTotal);
            ui->prb_Current->setValue(bytesReceived);
        }

        void CDownloadComponent::showFileInfo()
        {
            static const QString info("Files: %1 size: %2");
            ui->le_Info->setText(info.arg(m_remoteFiles.size()).arg(m_remoteFiles.getTotalFileSizeHumanReadable()));
        }
    } // ns
} // ns
