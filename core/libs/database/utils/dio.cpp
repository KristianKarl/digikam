/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dio.h"

// Qt includes

#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "imageinfo.h"
#include "albummanager.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "album.h"
#include "dmetadata.h"
#include "loadingcacheinterface.h"
#include "metadatasettings.h"
#include "scancontroller.h"
#include "thumbnailloadthread.h"
#include "iojobsmanager.h"
#include "collectionmanager.h"
#include "dnotificationwrapper.h"
#include "progressmanager.h"
#include "digikamapp.h"
#include "iojobdata.h"

namespace Digikam
{

SidecarFinder::SidecarFinder(const QList<QUrl>& files)
{
    foreach(const QUrl& url, files)
    {
        if (DMetadata::hasSidecar(url.toLocalFile()))
        {
            localFiles << DMetadata::sidecarUrl(url);
            localFileSuffixes << QLatin1String(".xmp");
            qCDebug(DIGIKAM_DATABASE_LOG) << "Detected a sidecar" << localFiles.last();
        }

        foreach(QString suffix, MetadataSettings::instance()->settings().sidecarExtensions)
        {
            suffix = QLatin1String(".") + suffix;
            QString sidecarName = url.toLocalFile() + suffix;

            if (QFileInfo::exists(sidecarName) && !localFiles.contains(QUrl::fromLocalFile(sidecarName)))
            {
                localFiles << QUrl::fromLocalFile(sidecarName);
                localFileSuffixes << suffix;
                qCDebug(DIGIKAM_DATABASE_LOG) << "Detected a sidecar" << localFiles.last();
            }
        }

        localFiles << url;
        localFileSuffixes << QString();
    }
}

// ------------------------------------------------------------------------------------------------

// TODO
// Groups should not be resolved in dio, it should be handled in views.
// This is already done for most things except for drag&drop, which is hard :)
GroupedImagesFinder::GroupedImagesFinder(const QList<ImageInfo>& source)
{
    QSet<qlonglong> ids;

    foreach(const ImageInfo& info, source)
    {
        ids << info.id();
    }

    infos.reserve(source.size());

    foreach(const ImageInfo& info, source)
    {
        infos << info;

        if (info.hasGroupedImages())
        {
            foreach(const ImageInfo& groupedImage, info.groupedImages())
            {
                if (ids.contains(groupedImage.id()))
                {
                    continue;
                }

                infos << groupedImage;
                ids << groupedImage.id();
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------

class DIOCreator
{
public:

    DIO object;
};

Q_GLOBAL_STATIC(DIOCreator, creator)

// ------------------------------------------------------------------------------------------------

DIO* DIO::instance()
{
    return &creator->object;
}

DIO::DIO()
{
}

DIO::~DIO()
{
}

void DIO::cleanUp()
{
}

// Album -> Album -----------------------------------------------------

void DIO::copy(PAlbum* const src, PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyAlbum, src, dest));
}

void DIO::move(PAlbum* const src, PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

#ifdef Q_OS_WIN
    AlbumManager::instance()->removeWatchedPAlbums(src);
#endif

    instance()->processJob(new IOJobData(IOJobData::MoveAlbum, src, dest));
}

// Images -> Album ----------------------------------------------------

void DIO::copy(const QList<ImageInfo>& infos, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyImage, infos, dest));
}

void DIO::move(const QList<ImageInfo>& infos, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::MoveImage, infos, dest));
}

// External files -> album --------------------------------------------

void DIO::copy(const QUrl& src, PAlbum* const dest)
{
    copy(QList<QUrl>() << src, dest);
}

void DIO::copy(const QList<QUrl>& srcList, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyFiles, srcList, dest));
}

void DIO::move(const QUrl& src, PAlbum* const dest)
{
    move(QList<QUrl>() << src, dest);
}

void DIO::move(const QList<QUrl>& srcList, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::MoveFiles, srcList, dest));
}

// Rename --------------------------------------------------------------

void DIO::rename(const ImageInfo& info, const QString& newName)
{
    instance()->processJob(new IOJobData(IOJobData::Rename, info, newName));
}

// Delete --------------------------------------------------------------

void DIO::del(const QList<ImageInfo>& infos, bool useTrash)
{
    instance()->processJob(new IOJobData(useTrash ? IOJobData::Trash 
                                                  : IOJobData::Delete, infos));
}

void DIO::del(const ImageInfo& info, bool useTrash)
{
    del(QList<ImageInfo>() << info, useTrash);
}

void DIO::del(PAlbum* const album, bool useTrash)
{
    if (!album)
    {
        return;
    }

#ifdef Q_OS_WIN
    AlbumManager::instance()->removeWatchedPAlbums(album);
#endif

    instance()->createJob(new IOJobData(useTrash ? IOJobData::Trash 
                                                 : IOJobData::Delete, album));
}

// ------------------------------------------------------------------------------------------------

void DIO::processJob(IOJobData* const data)
{
    const int operation = data->operation();

    if (operation == IOJobData::CopyImage || operation == IOJobData::MoveImage)
    {
        // this is a fast db operation, do here
        GroupedImagesFinder finder(data->imageInfos());
        data->setImageInfos(finder.infos);

        QStringList      filenames;
        QList<qlonglong> ids;

        foreach(const ImageInfo& info, data->imageInfos())
        {
            filenames << info.name();
            ids << info.id();
        }

        ScanController::instance()->hintAtMoveOrCopyOfItems(ids, data->destAlbum(), filenames);
    }
    else if (operation == IOJobData::CopyAlbum || operation == IOJobData::MoveAlbum)
    {
        ScanController::instance()->hintAtMoveOrCopyOfAlbum(data->srcAlbum(), data->destAlbum());
        createJob(data);
        return;
    }
    else if (operation == IOJobData::Delete || operation == IOJobData::Trash)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Number of files to be deleted:" << data->sourceUrls().count();
    }

    SidecarFinder finder(data->sourceUrls());
    data->setSourceUrls(finder.localFiles);

    if (operation == IOJobData::Rename)
    {
        PAlbum* const album = AlbumManager::instance()->findPAlbum(data->imageInfo().albumId());

        if (album)
        {
            ScanController::instance()->hintAtMoveOrCopyOfItem(data->imageInfo().id(),
                                                               album, data->destUrl().fileName());
        }

        for (int i = 0 ; i < finder.localFiles.length() ; ++i)
        {
            data->setDestUrl(finder.localFiles.at(i),
                             QUrl::fromLocalFile(data->destUrl().toLocalFile() +
                                                 finder.localFileSuffixes.at(i)));
        }
    }

    createJob(data);
}

void DIO::createJob(IOJobData* const data)
{
    if (data->sourceUrls().isEmpty())
    {
        delete data;
        return;
    }

    ProgressItem* item      = 0;
    IOJobsThread* jobThread = 0;
    const int operation     = data->operation();

    item = getProgressItem(operation);

    if (!item || item->totalCompleted())
    {
        QPair<QString, QString> itemStrings = getItemStrings(operation);

        if (!itemStrings.first.isEmpty())
        {
            item = ProgressManager::instance()->createProgressItem(itemStrings.first,
                                                                   itemStrings.second,
                                                                   QString(), true, false);
        }
    }

    jobThread = IOJobsManager::instance()->startIOJobs(data);

    connect(jobThread, SIGNAL(signalOneProccessed()),
            this, SLOT(slotOneProccessed()));

    connect(jobThread, SIGNAL(finished()),
            this, SLOT(slotResult()));

    if (operation == IOJobData::Rename)
    {
        connect(jobThread, SIGNAL(signalRenamed(QUrl)),
                this, SIGNAL(signalRenameSucceeded(QUrl)));

        connect(jobThread, SIGNAL(signalRenameFailed(QUrl)),
                this, SIGNAL(signalRenameFailed(QUrl)));
    }

    if (item)
    {
        item->setTotalItems(item->totalItems() + data->sourceUrls().count());

        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                jobThread, SLOT(slotCancel()));

        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                this, SLOT(slotCancel(ProgressItem*)));
    }
}

void DIO::slotResult()
{
    IOJobsThread* const jobThread = dynamic_cast<IOJobsThread*>(sender());

    if (!jobThread || !jobThread->jobData())
    {
        return;
    }

    IOJobData* const data = jobThread->jobData();
    const int operation   = data->operation();

    if (operation == IOJobData::MoveImage)
    {
        // update the image infos
        CoreDbAccess access;

        foreach(const ImageInfo& info, data->imageInfos())
        {
            if (data->processedUrls().contains(info.fileUrl()))
            {
                access.db()->moveItem(info.albumId(), info.name(), data->destAlbum()->id(), info.name());
            }
        }
    }
    else if (operation == IOJobData::Delete)
    {
        // Mark the images as obsolete and remove them
        // from their album and from the grouped
        PAlbum* const album = data->srcAlbum();
        CoreDbAccess access;

        if (album)
        {
            if (data->processedUrls().contains(album->fileUrl()))
            {
                // get all deleted albums
                QList<int> albumsToDelete;
                QList<qlonglong> imagesToRemove;

                addAlbumChildrenToList(albumsToDelete, album);

                foreach(int albumId, albumsToDelete)
                {
                    imagesToRemove << access.db()->getItemIDsInAlbum(albumId);
                }

                foreach(const qlonglong& imageId, imagesToRemove)
                {
                    access.db()->removeAllImageRelationsFrom(imageId, DatabaseRelation::Grouped);
                }

                access.db()->removeItemsPermanently(imagesToRemove, albumsToDelete);
            }
        }
        else
        {
            foreach(const ImageInfo& info, data->imageInfos())
            {
                if (data->processedUrls().contains(info.fileUrl()))
                {
                    access.db()->removeAllImageRelationsFrom(info.id(), DatabaseRelation::Grouped);
                    access.db()->removeItemsPermanently(QList<qlonglong>() << info.id(), QList<int>() << info.albumId());
                }
            }
        }
    }
    else if (operation == IOJobData::Rename)
    {
        // If we rename a file, the name changes. This is equivalent to a move.
        // Do this in database, too.
        if (data->processedUrls().contains(data->srcUrl()))
        {
            CoreDbAccess().db()->moveItem(data->imageInfo().albumId(),
                                          data->srcUrl().fileName(),
                                          data->imageInfo().albumId(),
                                          data->destUrl(data->srcUrl()).fileName());

            // delete thumbnail
            ThumbnailLoadThread::deleteThumbnail(data->srcUrl().toLocalFile());
            //LoadingCacheInterface::fileChanged(data->srcUrl().toLocalFile());
        }
    }

    if (jobThread->hasErrors() && operation != IOJobData::Rename)
    {
        // Pop-up a message about the error.
        QString errors = jobThread->errorsList().join(QLatin1String("\n"));
        DNotificationWrapper(QString(), errors, DigikamApp::instance(),
                             DigikamApp::instance()->windowTitle());
    }

    slotCancel(getProgressItem(operation));
}

void DIO::slotOneProccessed()
{
    IOJobsThread* const jobThread = dynamic_cast<IOJobsThread*>(sender());

    if (!jobThread || !jobThread->jobData())
    {
        return;
    }

    IOJobData* const data = jobThread->jobData();
    const int operation   = data->operation();
    QString path;

    // Scan folders for changes

    if (operation == IOJobData::CopyImage || operation == IOJobData::CopyAlbum ||
        operation == IOJobData::CopyFiles || operation == IOJobData::MoveImage ||
        operation == IOJobData::MoveAlbum || operation == IOJobData::MoveFiles)
    {
        path = data->destUrl().toLocalFile();
    }
    else if (operation == IOJobData::Delete || operation == IOJobData::Trash)
    {
        PAlbum* const album = data->srcAlbum();

        if (album)
        {
            PAlbum* const parent = dynamic_cast<PAlbum*>(album->parent());

            if (parent)
            {
                path = parent->fileUrl().toLocalFile();
            }
        }
        else if (!data->processedUrls().isEmpty())
        {
            QUrl url = data->processedUrls().last();
            path     = url.adjusted(QUrl::RemoveFilename).toLocalFile();
        }
    }

    if (!path.isEmpty())
    {
        ScanController::instance()->scheduleCollectionScanRelaxed(path);
    }

    ProgressItem* const item = getProgressItem(operation);

    if (item)
    {
        item->advance(1);
    }
}

ProgressItem* DIO::getProgressItem(int operation) const
{
    ProgressItem* item = 0;
    QString itemString = getItemStrings(operation).first;

    if (!itemString.isEmpty())
    {
        item = ProgressManager::instance()->findItembyId(itemString);
    }

    return item;
}

QPair<QString, QString> DIO::getItemStrings(int operation) const
{
    switch (operation)
    {
        case IOJobData::CopyAlbum:
        case IOJobData::CopyImage:
        case IOJobData::CopyFiles:
            return qMakePair(QLatin1String("DIOCopy"), i18n("Copy"));
        case IOJobData::MoveAlbum:
        case IOJobData::MoveImage:
        case IOJobData::MoveFiles:
            return qMakePair(QLatin1String("DIOMove"), i18n("Move"));
        case IOJobData::Trash:
            return qMakePair(QLatin1String("DIOTrash"), i18n("Trash"));
        case IOJobData::Delete:
            return qMakePair(QLatin1String("DIODelete"), i18n("Delete"));
        default:
            break;
    }

    return qMakePair(QString(), QString());
}

void DIO::slotCancel(ProgressItem* item)
{
    if (item)
    {
        item->setComplete();
    }
}

void DIO::addAlbumChildrenToList(QList<int>& list, Album* const album)
{
    // simple recursive helper function
    if (album)
    {
        if (!list.contains(album->id()))
        {
            list.append(album->id());
        }

        AlbumIterator it(album);

        while (it.current())
        {
            addAlbumChildrenToList(list, *it);
            ++it;
        }
    }
}

} // namespace Digikam
