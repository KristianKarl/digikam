/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-02-24
 * Description : Container for IOJob data.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "iojobdata.h"

// Qt includes

#include <QMutex>

// Local includes

#include "album.h"
#include "imageinfo.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN IOJobData::Private
{
public:

    explicit Private()
      : operation(Unknown),
        overwrite(false),
        srcAlbum(0),
        destAlbum(0)
    {
    }

    int              operation;

    bool             overwrite;

    PAlbum*          srcAlbum;
    PAlbum*          destAlbum;

    QMap<QUrl, QUrl> changeDestMap;
    QList<ImageInfo> imageInfoList;
    QList<QUrl>      sourceUrlList;

    QUrl             destUrl;

    QString          progressId;

    QMutex           mutex;
};

IOJobData::IOJobData(int operation,
                     const QList<ImageInfo>& infos,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation = operation;
    d->destAlbum = dest;

    setImageInfos(infos);

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     const QList<QUrl>& urls,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation     = operation;
    d->sourceUrlList = urls;
    d->destAlbum     = dest;

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     PAlbum* const src,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation = operation;
    d->srcAlbum  = src;
    d->destAlbum = dest;

    if (d->srcAlbum)
    {
        d->sourceUrlList << d->srcAlbum->fileUrl();
    }

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     const QList<QUrl>& urls,
                     const QUrl& dest)
    : d(new Private)
{
    d->operation     = operation;
    d->sourceUrlList = urls;
    d->destUrl       = dest;
}

IOJobData::IOJobData(int operation,
                     const ImageInfo& info,
                     const QString& newName,
                     bool overwrite)
    : d(new Private)
{
    d->operation = operation;
    d->overwrite = overwrite;

    setImageInfos(QList<ImageInfo>() << info);

    d->destUrl = info.fileUrl().adjusted(QUrl::RemoveFilename);
    d->destUrl.setPath(d->destUrl.path() + newName);
}

IOJobData::~IOJobData()
{
    delete d;
}

void IOJobData::setImageInfos(const QList<ImageInfo>& infos)
{
    d->imageInfoList = infos;

    d->sourceUrlList.clear();

    foreach(const ImageInfo& info, d->imageInfoList)
    {
        d->sourceUrlList << info.fileUrl();
    }
}

void IOJobData::setSourceUrls(const QList<QUrl>& urls)
{
    d->sourceUrlList = urls;
}

void IOJobData::setDestUrl(const QUrl& srcUrl,
                           const QUrl& destUrl)
{
    d->changeDestMap.insert(srcUrl, destUrl);
}

void IOJobData::setProgressId(const QString& id)
{
    d->progressId = id;
}

int IOJobData::operation() const
{
    return d->operation;
}

bool IOJobData::overwrite() const
{
    return d->overwrite;
}

PAlbum* IOJobData::srcAlbum() const
{
    return d->srcAlbum;
}

PAlbum* IOJobData::destAlbum() const
{
    return d->destAlbum;
}

QUrl IOJobData::destUrl(const QUrl& srcUrl) const
{
    if (srcUrl.isEmpty())
    {
        return d->destUrl;
    }

    return d->changeDestMap.value(srcUrl, d->destUrl);
}

QUrl IOJobData::getNextUrl() const
{
    d->mutex.lock();
    QUrl url;

    if (!d->sourceUrlList.isEmpty())
    {
        url = d->sourceUrlList.takeFirst();
    }

    d->mutex.unlock();
    return url;
}

QString IOJobData::getProgressId() const
{
    return d->progressId;
}

ImageInfo IOJobData::findImageInfo(const QUrl& url) const
{
    foreach(const ImageInfo& info, d->imageInfoList)
    {
        if (info.fileUrl() == url)
        {
            return info;
        }
    }

    return ImageInfo();
}

QList<QUrl> IOJobData::sourceUrls() const
{
    return d->sourceUrlList;
}

QList<ImageInfo> IOJobData::imageInfos() const
{
    return d->imageInfoList;
}

} // namespace Digikam
