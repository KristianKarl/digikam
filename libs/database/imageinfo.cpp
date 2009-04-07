/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling accesss to one image and associated data
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imageinfo.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QHash>

// KDE includes

#include <kdebug.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "imagelister.h"
#include "imageinfodata.h"
#include "imageinfocache.h"
#include "imagescanner.h"
#include "imageposition_p.h"
#include "imagecomments_p.h"

namespace Digikam
{

ImageInfoData::ImageInfoData()
{
    id                      = -1;
    albumId                 = -1;
    albumRootId             = -1;

    rating                  = -1;
    category                = DatabaseItem::UndefinedCategory;
    fileSize                = 0;

    defaultCommentCached    = false;
    ratingCached            = false;
    categoryCached          = false;
    formatCached            = false;
    creationDateCached      = false;
    modificationDateCached  = false;
    fileSizeCached          = false;
    imageSizeCached         = false;
    tagIdsCached            = false;
}

ImageInfo::ImageInfo()
         : m_data(0)
{
}

ImageInfo::ImageInfo(const ImageListerRecord &record)
{
    DatabaseAccess access;
    m_data                         = access.imageInfoCache()->infoForId(record.imageID);

    m_data->albumId                = record.albumID;
    m_data->albumRootId            = record.albumRootID;
    m_data->name                   = record.name;

    m_data->rating                 = record.rating;
    m_data->category               = record.category;
    m_data->format                 = record.format;
    m_data->creationDate           = record.creationDate;
    m_data->modificationDate       = record.modificationDate;
    m_data->fileSize               = record.fileSize;
    m_data->imageSize              = record.imageSize;

    m_data->ratingCached           = true;
    m_data->categoryCached         = true;
    m_data->formatCached           = true;
    m_data->creationDateCached     = true;
    m_data->modificationDateCached = true;
    m_data->fileSizeCached         = true;
    m_data->imageSizeCached        = true;
}

ImageInfo::ImageInfo(qlonglong ID)
{
    DatabaseAccess access;
    m_data = access.imageInfoCache()->infoForId(ID);
    // is this a newly created structure, need to populate?
    if (m_data->albumId == -1)
    {
        // retrieve immutable values now, the rest on demand
        ItemShortInfo info  = access.db()->getItemShortInfo(ID);
        if (info.id)
        {
            m_data->albumId     = info.albumID;
            m_data->albumRootId = info.albumRootID;
            m_data->name        = info.itemName;
        }
        else
        {
            // invalid image id
            ImageInfoData *olddata = m_data.unassign();
            if (olddata)
                access.imageInfoCache()->dropInfo(olddata);
            m_data = 0;
        }
    }
}

ImageInfo::ImageInfo(const KUrl &url)
{
    DatabaseAccess access;

    CollectionLocation location = CollectionManager::instance()->locationForUrl(url);
    if (location.isNull())
    {
        m_data = 0;
        return;
    }
    QString album = CollectionManager::instance()->album(url.directory());
    QString name  = url.fileName();

    // if needed, the two SQL calls can be consolidated into one by adding a method to AlbumDB
    int albumId = access.db()->getAlbumForPath(location.id(), album, false);
    if (albumId == -1)
    {
        m_data = 0;
        return;
    }

    int imageId = access.db()->getImageId(albumId, name);
    if (imageId == -1)
    {
        m_data = 0;
        return;
    }

    m_data = access.imageInfoCache()->infoForId(imageId);
    m_data->albumId     = albumId;
    m_data->albumRootId = location.id();
    m_data->name        = name;
    //m_data->url         = DatabaseUrl::fromAlbumAndName(name, album, location->albumRootPath());
}

ImageInfo::~ImageInfo()
{
    ImageInfoData *olddata = m_data.unassign();
    if (olddata)
        DatabaseAccess().imageInfoCache()->dropInfo(olddata);
}

ImageInfo::ImageInfo(const ImageInfo &info)
{
    m_data = info.m_data;
}

ImageInfo &ImageInfo::operator=(const ImageInfo &info)
{
    if (m_data == info.m_data)
        return *this;

    ImageInfoData *olddata = m_data.assign(info.m_data);
    if (olddata)
        DatabaseAccess().imageInfoCache()->dropInfo(olddata);

    return *this;
}

bool ImageInfo::isNull() const
{
    return !m_data;
}

bool ImageInfo::operator==(const ImageInfo &info) const
{
    if (m_data && info.m_data)
    {
        // not null, compare id
        return m_data->id == info.m_data->id;
    }
    else
    {
        // both null?
        return m_data == info.m_data;
    }
}

bool ImageInfo::operator<(const ImageInfo &info) const
{
    if (m_data)
    {
        if (info.m_data)
            // both not null, sort by id
            return m_data->id < info.m_data->id;
        else
            // only other is null, this is greater than
            return false;
    }
    else
    {
        // this is less than if the other is not null
        return info.m_data;
    }
}

uint ImageInfo::hash() const
{
    if (m_data)
        return ::qHash(m_data->id);
    else
        return ::qHash((int)0);
}

/**
 * Access rules for all methods in this class:
 * ImageInfoData members shall be accessed only under DatabaseAccess lock.
 * The id and albumId are the exception to this rule, as they are
 * primitive and will never change during the lifetime of an object.
 */
qlonglong ImageInfo::id() const
{
    return m_data ? m_data->id : -1;
}

int ImageInfo::albumId() const
{
    return m_data ? m_data->albumId : -1;
}

QString ImageInfo::name() const
{
    if (!m_data)
        return QString();

    DatabaseAccess access;
    return m_data->name;
}

uint ImageInfo::fileSize() const
{
    if (!m_data)
        return 0;

    DatabaseAccess access;
    if (!m_data->fileSizeCached)
    {
        QVariantList values = access.db()->getImagesFields(m_data->id, DatabaseFields::FileSize);
        if (!values.isEmpty())
            m_data.constCastData()->fileSize = values.first().toUInt();
        m_data.constCastData()->fileSizeCached = true;
    }

    return m_data->fileSize;
}

QString ImageInfo::comment() const
{
    if (!m_data)
        return QString();

    DatabaseAccess access;
    if (!m_data->defaultCommentCached)
    {
        ImageComments comments(access, m_data->id);
        m_data.constCastData()->defaultComment = comments.defaultComment();
        m_data.constCastData()->defaultCommentCached = true;
    }
    return m_data->defaultComment;
}

int ImageInfo::rating() const
{
    if (!m_data)
        return 0;

    DatabaseAccess access;
    if (!m_data->ratingCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::Rating);
        if (!values.isEmpty())
            m_data.constCastData()->rating = values.first().toInt();
        m_data.constCastData()->ratingCached = true;
    }
    return m_data->rating;
}

QString ImageInfo::format() const
{
    if (!m_data)
        return 0;

    DatabaseAccess access;
    if (!m_data->formatCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::Format);
        if (!values.isEmpty())
            m_data.constCastData()->format = values.first().toString();
        m_data.constCastData()->formatCached = true;
    }
    return m_data->format;
}

DatabaseItem::Category ImageInfo::category() const
{
    if (!m_data)
        return DatabaseItem::UndefinedCategory;

    DatabaseAccess access;
    if (!m_data->categoryCached)
    {
        QVariantList values = access.db()->getImagesFields(m_data->id, DatabaseFields::Category);
        if (!values.isEmpty())
            m_data.constCastData()->category = (DatabaseItem::Category)values.first().toInt();
        m_data.constCastData()->categoryCached = true;
    }
    return m_data->category;
}

QDateTime ImageInfo::dateTime() const
{
    if (!m_data)
        return QDateTime();

    DatabaseAccess access;
    if (!m_data->creationDateCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::CreationDate);
        if (!values.isEmpty())
            m_data.constCastData()->creationDate = values.first().toDateTime();
        m_data.constCastData()->creationDateCached = true;
    }
    return m_data->creationDate;
}

QDateTime ImageInfo::modDateTime() const
{
    if (!m_data)
        return QDateTime();

    DatabaseAccess access;
    if (!m_data->modificationDateCached)
    {
        QVariantList values = access.db()->getImagesFields(m_data->id, DatabaseFields::ModificationDate);
        if (!values.isEmpty())
            m_data.constCastData()->modificationDate = values.first().toDateTime();
        m_data.constCastData()->modificationDateCached = true;
    }
    return m_data->modificationDate;
}

QSize ImageInfo::dimensions() const
{
    if (!m_data)
        return QSize();

    DatabaseAccess access;
    if (!m_data->imageSizeCached)
    {
        QVariantList values = access.db()->getImageInformation(m_data->id, DatabaseFields::Width | DatabaseFields::Height);
        if (values.size() == 2)
            m_data.constCastData()->imageSize = QSize(values[0].toInt(), values[1].toInt());
        m_data.constCastData()->imageSizeCached = true;
    }
    return m_data->imageSize;
}

QList<int> ImageInfo::tagIds() const
{
    if (!m_data)
        return QList<int>();

    DatabaseAccess access;
    if (!m_data->tagIdsCached)
    {
        m_data.constCastData()->tagIds = access.db()->getItemTagIDs(m_data->id);
        m_data.constCastData()->imageSizeCached = true;
    }
    return m_data->tagIds;
}

DatabaseUrl ImageInfo::databaseUrl() const
{
    if (!m_data)
        return DatabaseUrl();

    DatabaseAccess access;

    QString album = access.imageInfoCache()->albumName(access, m_data->albumId);
    QString albumRoot = CollectionManager::instance()->albumRootPath(m_data->albumRootId);

    return DatabaseUrl::fromAlbumAndName(m_data->name, album, albumRoot, m_data->albumRootId);
}

KUrl ImageInfo::fileUrl() const
{
    return KUrl::fromPath(filePath());
}

QString ImageInfo::filePath() const
{
    if (!m_data)
        return QString();

    DatabaseAccess access;

    QString album = access.imageInfoCache()->albumName(access, m_data->albumId);
    QString albumRoot = CollectionManager::instance()->albumRootPath(m_data->albumRootId);

    if (album == "/")
        return albumRoot + album + m_data->name;
    else
        return albumRoot + album + "/" + m_data->name;
}

ImageComments ImageInfo::imageComments(DatabaseAccess &access) const
{
    if (!m_data)
        return ImageComments();

    return ImageComments(access, m_data->id);
}

ImagePosition ImageInfo::imagePosition() const
{
    if (!m_data)
        return ImagePosition();

    return ImagePosition(m_data->id);
}

ImageCommonContainer ImageInfo::imageCommonContainer() const
{
    if (!m_data)
        return ImageCommonContainer();

    ImageCommonContainer container;
    ImageScanner::fillCommonContainer(m_data->id, &container);
    return container;
}

ImageMetadataContainer ImageInfo::imageMetadataContainer() const
{
    if (!m_data)
        return ImageMetadataContainer();

    ImageMetadataContainer container;
    ImageScanner::fillMetadataContainer(m_data->id, &container);
    return container;
}

PhotoInfoContainer ImageInfo::photoInfoContainer() const
{
    if (!m_data)
        return PhotoInfoContainer();

    ImageMetadataContainer meta = imageMetadataContainer();
    PhotoInfoContainer photoInfo;

    photoInfo.make            = meta.make;
    photoInfo.model           = meta.model;
    photoInfo.lens            = meta.lens;
    photoInfo.exposureTime    = meta.exposureTime;
    photoInfo.exposureMode    = meta.exposureMode;
    photoInfo.exposureProgram = meta.exposureProgram;
    photoInfo.aperture        = meta.aperture;
    photoInfo.focalLength     = meta.focalLength;
    photoInfo.focalLength35mm = meta.focalLength35;
    photoInfo.sensitivity     = meta.sensitivity;
    photoInfo.flash           = meta.flashMode;
    photoInfo.whiteBalance    = meta.whiteBalance;
    photoInfo.dateTime        = dateTime();

    return photoInfo;
}

void ImageInfo::setRating(int value)
{
    if (!m_data)
        return;

    DatabaseAccess access;
    access.db()->changeImageInformation(m_data->id, QVariantList() << value, DatabaseFields::Rating);

    m_data->rating = value;
    m_data.constCastData()->ratingCached = true;
}

void ImageInfo::setDateTime(const QDateTime& dateTime)
{
    if (!m_data)
        return;

    if (dateTime.isValid())
    {
        DatabaseAccess access;
        access.db()->changeImageInformation(m_data->id, QVariantList() << dateTime, DatabaseFields::CreationDate);

        m_data->creationDate = dateTime;
        m_data.constCastData()->creationDateCached = true;
    }
}

void ImageInfo::setTag(int tagID)
{
    DatabaseAccess access;
    access.db()->addItemTag(m_data->id, tagID);
}

void ImageInfo::removeTag(int tagID)
{
    DatabaseAccess access;
    access.db()->removeItemTag(m_data->id, tagID);
}

void ImageInfo::removeAllTags()
{
    DatabaseAccess access;
    access.db()->removeItemAllTags(m_data->id);
}

void ImageInfo::addTagPaths(const QStringList &tagPaths)
{
    DatabaseAccess access;
    QList<int> list = access.db()->getTagsFromTagPaths(tagPaths, false);
    for (int i=0; i<list.count(); ++i)
        access.db()->addItemTag(m_data->id, list[i]);
}

ImageInfo ImageInfo::copyItem(int dstAlbumID, const QString &dstFileName)
{
    DatabaseAccess access;
    //kDebug(50003) << "ImageInfo::copyItem " << m_data->albumId << " " << m_data->name << " to " << dstAlbumID << " " << dstFileName << endl;

    if (dstAlbumID == m_data->albumId && dstFileName == m_data->name)
        return (*this);

    int id = access.db()->copyItem(m_data->albumId, m_data->name, dstAlbumID, dstFileName);

    if (id == -1)
        return ImageInfo();

    return ImageInfo(id);
}

}  // namespace Digikam
