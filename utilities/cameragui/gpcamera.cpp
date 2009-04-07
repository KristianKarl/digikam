/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-21
 * Description : Gphoto2 camera interface
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "gpcamera.h"

// C++ includes

#include <cstdio>
#include <iostream>

// Qt includes

#include <QString>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QDateTime>

// KDE includes

#include <kcodecs.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "config-digikam.h"

#ifdef ENABLE_GPHOTO2

// LibGphoto2 includes

extern "C"
{
#include <gphoto2.h>
}

#endif /* ENABLE_GPHOTO2 */

namespace Digikam
{

class GPStatus
{

public:

    GPStatus()
    {
#ifdef ENABLE_GPHOTO2
        context = gp_context_new();
        cancel  = false;
        gp_context_set_cancel_func(context, cancel_func, 0);
#endif /* ENABLE_GPHOTO2 */
    }

    ~GPStatus()
    {
#ifdef ENABLE_GPHOTO2
        gp_context_unref(context);
        cancel = false;
#endif /* ENABLE_GPHOTO2 */
    }

    static bool  cancel;

#ifdef ENABLE_GPHOTO2
    GPContext   *context;

    static GPContextFeedback cancel_func(GPContext *, void *)
    {
        return (cancel ? GP_CONTEXT_FEEDBACK_CANCEL :
                GP_CONTEXT_FEEDBACK_OK);
    }
#endif /* ENABLE_GPHOTO2 */
};

bool GPStatus::cancel = false;

class GPCameraPrivate
{

public:

    GPCameraPrivate()
    {
#ifdef ENABLE_GPHOTO2
        status            = 0;
        camera            = 0;
        cameraInitialized = false;
#endif /* ENABLE_GPHOTO2 */
    }

#ifdef ENABLE_GPHOTO2
    bool             cameraInitialized;

    Camera          *camera;

    CameraAbilities  cameraAbilities;

    GPStatus        *status;
#endif /* ENABLE_GPHOTO2 */
};

GPCamera::GPCamera(const QString& title, const QString& model,
                   const QString& port, const QString& path)
        : DKCamera(title, model, port, path),
          d(new GPCameraPrivate)
{
}

GPCamera::~GPCamera()
{
#ifdef ENABLE_GPHOTO2
    if (d->camera)
    {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }
#endif /* ENABLE_GPHOTO2 */

    delete d;
}

QByteArray GPCamera::cameraMD5ID()
{
    QByteArray md5data;

#ifdef ENABLE_GPHOTO2
    QString camData;
    // We don't use camera title from digiKam settings panel to compute MD5 fingerprint,
    // because it can be changed by users between session.
    camData.append(model());
    camData.append(port());
    camData.append(path());
    KMD5 md5(camData.toUtf8());
    md5data = md5.hexDigest();
#endif /* ENABLE_GPHOTO2 */

    return md5data;
}

bool GPCamera::doConnect()
{
#ifdef ENABLE_GPHOTO2
    int errorCode;

    // -- first step - setup the camera --------------------

    if (d->camera)
    {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }

    CameraAbilitiesList *abilList;
    GPPortInfoList      *infoList;
    GPPortInfo           info;

    gp_camera_new(&d->camera);

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus();

    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, d->status->context);
    gp_port_info_list_new(&infoList);
    gp_port_info_list_load(infoList);

    delete d->status;
    d->status = 0;

    int modelNum = -1, portNum = -1;
    modelNum = gp_abilities_list_lookup_model(abilList, m_model.toLatin1());
    portNum  = gp_port_info_list_lookup_path (infoList, m_port.toLatin1());

    gp_abilities_list_get_abilities(abilList, modelNum, &d->cameraAbilities);

    errorCode = gp_camera_set_abilities(d->camera, d->cameraAbilities);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to set camera Abilities!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_camera_unref(d->camera);
        d->camera = 0;
        gp_abilities_list_free(abilList);
        gp_port_info_list_free(infoList);
        return false;
    }

    if (m_model != "Directory Browse")
    {
        gp_port_info_list_get_info(infoList, portNum, &info);
        errorCode = gp_camera_set_port_info(d->camera, info);
        if (errorCode != GP_OK)
        {
            kDebug(50003) << "Failed to set camera port!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_camera_unref(d->camera);
            d->camera = 0;
            gp_abilities_list_free (abilList);
            gp_port_info_list_free (infoList);
            return false;
        }
    }

    gp_abilities_list_free (abilList);
    gp_port_info_list_free (infoList);

    if (d->cameraAbilities.file_operations &
        GP_FILE_OPERATION_PREVIEW)
        m_thumbnailSupport = true;

    if (d->cameraAbilities.file_operations &
        GP_FILE_OPERATION_DELETE)
        m_deleteSupport = true;

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_PUT_FILE)
        m_uploadSupport = true;

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_MAKE_DIR)
        m_mkDirSupport = true;

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_REMOVE_DIR)
        m_delDirSupport = true;

    if (d->cameraAbilities.operations &
        GP_OPERATION_CAPTURE_IMAGE)
        m_captureImageSupport = true;

    // -- Try and initialize the camera to see if its connected -----------------

    d->status = new GPStatus();

    errorCode = gp_camera_init(d->camera, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to initialize camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_camera_unref(d->camera);
        d->camera = 0;
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;
    d->cameraInitialized = true;

    return true;
#else
    return false;
#endif /* ENABLE_GPHOTO2 */
}

void GPCamera::cancel()
{
#ifdef ENABLE_GPHOTO2
    if (!d->status)
        return;
    d->status->cancel = true;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::getFreeSpace(unsigned long& kBSize, unsigned long& kBAvail)
{
#ifdef ENABLE_GPHOTO2
    // NOTE: This method depends of libgphoto2 2.4.0

    int                       nrofsinfos;
    CameraStorageInformation *sinfos;

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }
    d->status = new GPStatus();

    int errorCode = gp_camera_get_storageinfo (d->camera, &sinfos, &nrofsinfos, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Getting storage information not supported for this camera!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    for (int i = 0 ; i < nrofsinfos ; ++i)
    {
        if (sinfos[i].fields & GP_STORAGEINFO_FILESYSTEMTYPE)
        {
            switch (sinfos[i].fstype)
            {
                case GP_STORAGEINFO_FST_DCF:       // Camera layout (DCIM)
                {
                    if (sinfos[i].fields & GP_STORAGEINFO_LABEL)
                        kDebug(50003) << "Storage label: " << QString(sinfos[i].label) << endl;
                    if (sinfos[i].fields & GP_STORAGEINFO_DESCRIPTION)
                        kDebug(50003) << "Storage description: " << QString(sinfos[i].description) << endl;
                    if (sinfos[i].fields & GP_STORAGEINFO_BASE)
                        kDebug(50003) << "Storage base-dir: " << QString(sinfos[i].basedir) << endl;

                    if (sinfos[i].fields & GP_STORAGEINFO_ACCESS)
                    {
                        switch (sinfos[i].access)
                        {
                            case GP_STORAGEINFO_AC_READWRITE:
                                kDebug(50003) << "Storage access: R/W" << endl;
                                break;
                            case GP_STORAGEINFO_AC_READONLY:
                                kDebug(50003) << "Storage access: RO" << endl;
                                break;
                            case GP_STORAGEINFO_AC_READONLY_WITH_DELETE:
                                kDebug(50003) << "Storage access: RO + Del" << endl;
                                break;
                            default:
                                break;
                        }
                    }

                    if (sinfos[i].fields & GP_STORAGEINFO_STORAGETYPE)
                    {
                        switch (sinfos[i].type)
                        {
                            case GP_STORAGEINFO_ST_FIXED_ROM:
                                kDebug(50003) << "Storage type: fixed ROM" << endl;
                                break;
                            case GP_STORAGEINFO_ST_REMOVABLE_ROM:
                                kDebug(50003) << "Storage type: removable ROM" << endl;
                                break;
                            case GP_STORAGEINFO_ST_FIXED_RAM:
                                kDebug(50003) << "Storage type: fixed RAM" << endl;
                                break;
                            case GP_STORAGEINFO_ST_REMOVABLE_RAM:
                                kDebug(50003) << "Storage type: removable RAM" << endl;
                                break;
                            case GP_STORAGEINFO_ST_UNKNOWN:
                            default:
                                kDebug(50003) << "Storage type: unknown" << endl;
                                break;
                        }
                    }

                    if (sinfos[i].fields & GP_STORAGEINFO_MAXCAPACITY)
                    {
                        kBSize = sinfos[i].capacitykbytes;
                        kDebug(50003) << "Storage capacity: " << kBSize << endl;
                    }
                    else
                    {
                        delete d->status;
                        d->status = 0;
                        return false;
                    }

                    if (sinfos[i].fields & GP_STORAGEINFO_FREESPACEKBYTES)
                    {
                        kBAvail = sinfos[i].freekbytes;
                        kDebug(50003) << "Storage free-space: " << kBAvail << endl;
                        return true;
                    }
                    else
                    {
                        delete d->status;
                        d->status = 0;
                        return false;
                    }
                    break;
                }
                case GP_STORAGEINFO_FST_UNDEFINED:
                case GP_STORAGEINFO_FST_GENERICFLAT:
                case GP_STORAGEINFO_FST_GENERICHIERARCHICAL:
                default:
                    break;
            }
        }
    }

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(kBSize);
    Q_UNUSED(kBAvail);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::getPreview(QImage& preview)
{
#ifdef ENABLE_GPHOTO2
    int                errorCode;
    CameraFile        *cfile;
    const char        *data;
    unsigned long int  size;

    gp_file_new(&cfile);

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_capture_preview(d->camera, cfile, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to initialize camera preview mode!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;

    errorCode = gp_file_get_data_and_size(cfile, &data, &size);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get preview from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    preview.loadFromData((const uchar*) data, (uint) size);

    gp_file_unref(cfile);
    return true;
#else
    Q_UNUSED(preview);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::capture(GPItemInfo& itemInfo)
{
#ifdef ENABLE_GPHOTO2
    int            errorCode;
    CameraFilePath path;

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_capture(d->camera, GP_CAPTURE_IMAGE, &path, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to take camera capture!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    // Get new camera item information.

    itemInfo.folder = QString(path.folder);
    itemInfo.name   = QString(path.name);

    CameraFileInfo info;
    errorCode = gp_camera_file_get_info(d->camera, QFile::encodeName(itemInfo.folder),
                                        QFile::encodeName(itemInfo.name), &info,
                                        d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera item information!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    itemInfo.mtime            = QDateTime();
    itemInfo.mime             = "";
    itemInfo.size             = -1;
    itemInfo.width            = -1;
    itemInfo.height           = -1;
    itemInfo.downloaded       = GPItemInfo::DownloadUnknow;
    itemInfo.readPermissions  = -1;
    itemInfo.writePermissions = -1;

    /* The mime type returned by Gphoto2 is dummy with all RAW files.
    if (info.file.fields & GP_FILE_INFO_TYPE)
        itemInfo.mime = info.file.type;*/

    itemInfo.mime = mimeType(itemInfo.name.section('.', -1).toLower());

    if (info.file.fields & GP_FILE_INFO_MTIME)
        itemInfo.mtime = QDateTime::fromTime_t(info.file.mtime);

    if (info.file.fields & GP_FILE_INFO_SIZE)
        itemInfo.size = info.file.size;

    if (info.file.fields & GP_FILE_INFO_WIDTH)
        itemInfo.width = info.file.width;

    if (info.file.fields & GP_FILE_INFO_HEIGHT)
        itemInfo.height = info.file.height;

    if (info.file.fields & GP_FILE_INFO_STATUS)
    {
        if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
            itemInfo.downloaded = GPItemInfo::DownloadedYes;
        else
            itemInfo.downloaded = GPItemInfo::DownloadedNo;
    }

    if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
    {
        if (info.file.permissions & GP_FILE_PERM_READ)
            itemInfo.readPermissions = 1;
        else
            itemInfo.readPermissions = 0;
        if (info.file.permissions & GP_FILE_PERM_DELETE)
            itemInfo.writePermissions = 1;
        else
            itemInfo.writePermissions = 0;
    }

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(itemInfo);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

void GPCamera::getAllFolders(const QString& rootFolder, QStringList& folderList)
{
    QStringList subfolders;
    getSubFolders(rootFolder, subfolders);

    for (QStringList::iterator it = subfolders.begin();
         it != subfolders.end(); ++it)
    {
        *it = rootFolder + QString(rootFolder.endsWith('/') ? "" : "/") + (*it);
        folderList.append(*it);
    }

    for (QStringList::const_iterator it = subfolders.constBegin();
         it != subfolders.constEnd(); ++it)
    {
        getAllFolders(*it, folderList);
    }
}

bool GPCamera::getSubFolders(const QString& folder, QStringList& subFolderList)
{
#ifdef ENABLE_GPHOTO2
    int         errorCode;
    CameraList *clist;
    gp_list_new(&clist);

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }
    d->status = new GPStatus();

    errorCode = gp_camera_folder_list_folders(d->camera, QFile::encodeName(folder), clist, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get folders list from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;

    int count = gp_list_count(clist);
    for (int i = 0 ; i < count ; ++i)
    {
        const char* subFolder;
        errorCode = gp_list_get_name(clist, i, &subFolder);
        if (errorCode != GP_OK)
        {
            kDebug(50003) << "Failed to get folder name from camera!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            return false;
        }

        subFolderList.append(QFile::decodeName(subFolder));
    }

    gp_list_unref(clist);
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(subFolderList);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::getItemsList(const QString& folder, QStringList& itemsList)
{
#ifdef ENABLE_GPHOTO2
    int         errorCode;
    CameraList *clist;
    const char *cname;

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }
    d->status = new GPStatus;

    gp_list_new(&clist);

    errorCode = gp_camera_folder_list_files(d->camera, QFile::encodeName(folder), clist, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get folder files list from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        delete d->status;
        d->status = 0;
        return false;
    }

    int count = gp_list_count(clist);
    for (int i = 0 ; i < count ; ++i)
    {
        errorCode = gp_list_get_name(clist, i, &cname);
        if (errorCode != GP_OK)
        {
            kDebug(50003) << "Failed to get file name from camera!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            delete d->status;
            d->status = 0;
            return false;
        }

        itemsList.append(QFile::decodeName(cname));
    }

    gp_list_unref(clist);

    delete d->status;
    d->status = 0;

    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemsList);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::getItemsInfoList(const QString& folder, GPItemInfoList& items, bool /*getImageDimensions*/)
{
#ifdef ENABLE_GPHOTO2
    int         errorCode;
    CameraList *clist;
    const char *cname;

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }
    d->status = new GPStatus;

    gp_list_new(&clist);

    errorCode = gp_camera_folder_list_files(d->camera, QFile::encodeName(folder), clist, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get folder files list from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        delete d->status;
        d->status = 0;
        return false;
    }

    int count = gp_list_count(clist);
    for (int i = 0 ; i < count ; ++i)
    {
        errorCode = gp_list_get_name(clist, i, &cname);
        if (errorCode != GP_OK)
        {
            kDebug(50003) << "Failed to get file name from camera!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            delete d->status;
            d->status = 0;
            return false;
        }

        GPItemInfo itemInfo;

        itemInfo.name   = QFile::decodeName(cname);
        itemInfo.folder = folder;

        CameraFileInfo info;
        gp_camera_file_get_info(d->camera, QFile::encodeName(folder),
                                cname, &info, d->status->context);

        itemInfo.mtime            = QDateTime();
        itemInfo.mime             = "";
        itemInfo.size             = -1;
        itemInfo.width            = -1;
        itemInfo.height           = -1;
        itemInfo.downloaded       = GPItemInfo::DownloadUnknow;
        itemInfo.readPermissions  = -1;
        itemInfo.writePermissions = -1;

        /* The mime type returned by Gphoto2 is dummy with all RAW files.
        if (info.file.fields & GP_FILE_INFO_TYPE)
            itemInfo.mime = info.file.type;*/

        itemInfo.mime = mimeType(itemInfo.name.section('.', -1).toLower());

        if (info.file.fields & GP_FILE_INFO_MTIME)
            itemInfo.mtime = QDateTime::fromTime_t(info.file.mtime);

        if (info.file.fields & GP_FILE_INFO_SIZE)
            itemInfo.size = info.file.size;

        if (info.file.fields & GP_FILE_INFO_WIDTH)
            itemInfo.width = info.file.width;

        if (info.file.fields & GP_FILE_INFO_HEIGHT)
            itemInfo.height = info.file.height;

        if (info.file.fields & GP_FILE_INFO_STATUS)
        {
            if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
                itemInfo.downloaded = GPItemInfo::DownloadedYes;
        }

        if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
        {
            if (info.file.permissions & GP_FILE_PERM_READ)
                itemInfo.readPermissions = 1;
            else
                itemInfo.readPermissions = 0;
            if (info.file.permissions & GP_FILE_PERM_DELETE)
                itemInfo.writePermissions = 1;
            else
                itemInfo.writePermissions = 0;
        }

        items.append(itemInfo);
    }

    gp_list_unref(clist);

    delete d->status;
    d->status = 0;

    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(items);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)
{
#ifdef ENABLE_GPHOTO2
    int                errorCode;
    CameraFile        *cfile;
    const char        *data;
    unsigned long int  size;

    gp_file_new(&cfile);

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_file_get(d->camera, QFile::encodeName(folder),
                                   QFile::encodeName(itemName),
                                   GP_FILE_TYPE_PREVIEW,
                                   cfile, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;

    errorCode = gp_file_get_data_and_size(cfile, &data, &size);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get thumbnail from camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    thumbnail.loadFromData((const uchar*) data, (uint) size);

    gp_file_unref(cfile);
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(thumbnail);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::getExif(const QString& folder, const QString& itemName,
                       char **edata, int& esize)
{
#ifdef ENABLE_GPHOTO2
    int                errorCode;
    CameraFile        *cfile;
    const char        *data;
    unsigned long int  size;

    gp_file_new(&cfile);

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_file_get(d->camera, QFile::encodeName(folder),
                                   QFile::encodeName(itemName),
                                   GP_FILE_TYPE_EXIF,
                                   cfile, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;

    errorCode = gp_file_get_data_and_size(cfile, &data, &size);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get Exif data from camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    *edata = new char[size];
    esize  = size;
    memcpy(*edata, data, size);

    gp_file_unref(cfile);
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(edata);
    Q_UNUSED(esize);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::downloadItem(const QString& folder, const QString& itemName,
                            const QString& saveFile)
{
#ifdef ENABLE_GPHOTO2
    int         errorCode;
    CameraFile *cfile;

    gp_file_new(&cfile);

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_file_get(d->camera, QFile::encodeName(folder),
                                   QFile::encodeName(itemName),
                                   GP_FILE_TYPE_NORMAL, cfile,
                                   d->status->context);
    if ( errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;

    errorCode = gp_file_save(cfile, QFile::encodeName(saveFile));
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to save camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    gp_file_unref(cfile);
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(saveFile);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::setLockItem(const QString& folder, const QString& itemName, bool lock)
{
#ifdef ENABLE_GPHOTO2
    int errorCode;
    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    CameraFileInfo info;
    errorCode = gp_camera_file_get_info(d->camera, QFile::encodeName(folder),
                                QFile::encodeName(itemName), &info, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera item properties!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
    {
        if (lock)
        {
            // Lock the file to set read only flag
            info.file.permissions = (CameraFilePermissions)GP_FILE_PERM_READ;
        }
        else
        {
            // Unlock the file to set read/write flag
            info.file.permissions = (CameraFilePermissions)(GP_FILE_PERM_READ | GP_FILE_PERM_DELETE);
        }
    }

    // Some gphoto2 drivers need to have only the right flag at on to process properties update in camera.
    info.file.fields    = GP_FILE_INFO_PERMISSIONS;
    info.preview.fields = GP_FILE_INFO_NONE;
    info.audio.fields   = GP_FILE_INFO_NONE;

    errorCode = gp_camera_file_set_info(d->camera, QFile::encodeName(folder),
                                        QFile::encodeName(itemName), info, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to set camera item lock properties!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(lock);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::deleteItem(const QString& folder, const QString& itemName)
{
#ifdef ENABLE_GPHOTO2
    int errorCode;
    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_file_delete(d->camera, QFile::encodeName(folder),
                                      QFile::encodeName(itemName),
                                      d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to delete camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

// recursively delete all items
bool GPCamera::deleteAllItems(const QString& folder)
{
#ifdef ENABLE_GPHOTO2
    int         errorCode;
    QStringList folderList;

    // Get all subfolders in this folder
    getSubFolders(folder, folderList);

    if (folderList.count() > 0)
    {
        for (int i = 0 ; i < folderList.count() ; ++i)
        {
            QString subFolder(folder);

            if (!subFolder.endsWith('/'))
                subFolder += '/';

            subFolder += folderList[i];
            deleteAllItems(subFolder);
        }
    }

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_folder_delete_all(d->camera, QFile::encodeName(folder),
                                            d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to delete camera folder!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(folder);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::uploadItem(const QString& folder, const QString& itemName, const QString& localFile,
                          GPItemInfo& itemInfo, bool /*getImageDimensions*/)
{
#ifdef ENABLE_GPHOTO2
    int         errorCode;
    CameraFile *cfile;

    errorCode = gp_file_new(&cfile);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to init new camera file instance!" << endl;
        printGphotoErrorDescription(errorCode);
        return false;
    }

    errorCode = gp_file_open(cfile, QFile::encodeName(localFile));
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to open file!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    errorCode = gp_file_set_name(cfile, QFile::encodeName(itemName));
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to rename item from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_folder_put_file(d->camera,
                                          QFile::encodeName(folder),
                                          cfile,
                                          d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to upload item to camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete d->status;
        d->status = 0;
        return false;
    }

    // Get new camera item information.

    itemInfo.name   = itemName;
    itemInfo.folder = folder;

    CameraFileInfo info;
    errorCode = gp_camera_file_get_info(d->camera, QFile::encodeName(folder),
                                        QFile::encodeName(itemName), &info, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera item information!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete d->status;
        d->status = 0;
        return false;
    }

    itemInfo.mtime            = QDateTime();
    itemInfo.mime             = "";
    itemInfo.size             = -1;
    itemInfo.width            = -1;
    itemInfo.height           = -1;
    itemInfo.downloaded       = GPItemInfo::DownloadUnknow;
    itemInfo.readPermissions  = -1;
    itemInfo.writePermissions = -1;

    /* The mime type returned by Gphoto2 is dummy with all RAW files.
    if (info.file.fields & GP_FILE_INFO_TYPE)
        itemInfo.mime = info.file.type;*/

    itemInfo.mime = mimeType(itemInfo.name.section('.', -1).toLower());

    if (info.file.fields & GP_FILE_INFO_MTIME)
        itemInfo.mtime = QDateTime::fromTime_t(info.file.mtime);

    if (info.file.fields & GP_FILE_INFO_SIZE)
        itemInfo.size = info.file.size;

    if (info.file.fields & GP_FILE_INFO_WIDTH)
        itemInfo.width = info.file.width;

    if (info.file.fields & GP_FILE_INFO_HEIGHT)
        itemInfo.height = info.file.height;

    if (info.file.fields & GP_FILE_INFO_STATUS)
    {
        if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
            itemInfo.downloaded = GPItemInfo::DownloadedYes;
        else
            itemInfo.downloaded = GPItemInfo::DownloadedNo;
    }

    if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
    {
        if (info.file.permissions & GP_FILE_PERM_READ)
            itemInfo.readPermissions = 1;
        else
            itemInfo.readPermissions = 0;
        if (info.file.permissions & GP_FILE_PERM_DELETE)
            itemInfo.writePermissions = 1;
        else
            itemInfo.writePermissions = 0;
    }

    gp_file_unref(cfile);
    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(localFile);
    Q_UNUSED(itemInfo);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::cameraSummary(QString& summary)
{
#ifdef ENABLE_GPHOTO2
    int        errorCode;
    CameraText sum;

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_get_summary(d->camera, &sum, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera summary!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    summary =  i18n("Title: <b>%1</b><br/>"
                    "Model: <b>%2</b><br/>"
                    "Port: <b>%3</b><br/>"
                    "Path: <b>%4</b><br/><br/>",
                    title(),
                    model(),
                    port(),
                    path());

    summary += i18n("Thumbnails: <b>%1</b><br/>"
                    "Capture image: <b>%2</b><br/>"
                    "Delete items: <b>%3</b><br/>"
                    "Upload items: <b>%4</b><br/>"
                    "Create directories: <b>%5</b><br/>"
                    "Delete Directories: <b>%6</b><br/><br/>",
                    thumbnailSupport()    ? i18n("yes") : i18n("no"),
                    captureImageSupport() ? i18n("yes") : i18n("no"),
                    deleteSupport()       ? i18n("yes") : i18n("no"),
                    uploadSupport()       ? i18n("yes") : i18n("no"),
                    mkDirSupport()        ? i18n("yes") : i18n("no"),
                    delDirSupport()       ? i18n("yes") : i18n("no"));

    summary.append(QString(sum.text));

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(summary);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::cameraManual(QString& manual)
{
#ifdef ENABLE_GPHOTO2
    int        errorCode;
    CameraText man;

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_get_manual(d->camera, &man, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get camera manual!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    manual = QString(man.text);

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(manual);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

bool GPCamera::cameraAbout(QString& about)
{
#ifdef ENABLE_GPHOTO2
    int        errorCode;
    CameraText abt;

    if (d->status)
    {
        delete d->status;
        d->status = 0;
    }

    d->status = new GPStatus;

    errorCode = gp_camera_get_about(d->camera, &abt, d->status->context);
    if (errorCode != GP_OK)
    {
        kDebug(50003) << "Failed to get information about camera!" << endl;
        printGphotoErrorDescription(errorCode);
        delete d->status;
        d->status = 0;
        return false;
    }

    about = QString(abt.text);
    about.append(i18n("\n\nTo report problems about this driver, please contact "
                      "the gphoto2 team at:\n\nhttp://gphoto.org/bugs"));

    delete d->status;
    d->status = 0;
    return true;
#else
    Q_UNUSED(about);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

// -- Static methods ---------------------------------------------------------------------

void GPCamera::printGphotoErrorDescription(int errorCode)
{
#ifdef ENABLE_GPHOTO2
    kDebug(50003) << "Libgphoto2 error: " << gp_result_as_string(errorCode)
                  << " (" << errorCode << ")" << endl;
#else
    Q_UNUSED(errorCode);
#endif /* ENABLE_GPHOTO2 */
}

void GPCamera::getSupportedCameras(int& count, QStringList& clist)
{
#ifdef ENABLE_GPHOTO2
    clist.clear();
    count = 0;

    CameraAbilitiesList *abilList;
    CameraAbilities      abil;
    GPContext           *context;

    context = gp_context_new();

    gp_abilities_list_new( &abilList );
    gp_abilities_list_load( abilList, context );

    count = gp_abilities_list_count( abilList );
    if ( count < 0 )
    {
        kDebug(50003) << "Failed to get list of cameras!" << endl;
        printGphotoErrorDescription(count);
        gp_context_unref( context );
        return;
    }
    else
    {
        for (int i = 0 ; i < count ; ++i)
        {
            gp_abilities_list_get_abilities( abilList, i, &abil );
            const char *cname = abil.model;
            clist.append( QString( cname ) );
        }
    }

    gp_abilities_list_free( abilList );
    gp_context_unref( context );
#else
    Q_UNUSED(count);
    Q_UNUSED(clist);
#endif /* ENABLE_GPHOTO2 */
}

void GPCamera::getSupportedPorts(QStringList& plist)
{
#ifdef ENABLE_GPHOTO2
    GPPortInfoList *list;
    GPPortInfo      info;

    plist.clear();

    gp_port_info_list_new( &list );
    gp_port_info_list_load( list );

    int numPorts = gp_port_info_list_count( list );
    if ( numPorts < 0)
    {
        kDebug(50003) << "Failed to get list of port!" << endl;
        printGphotoErrorDescription(numPorts);
        gp_port_info_list_free( list );
        return;
    }
    else
    {
        for (int i = 0 ; i < numPorts ; ++i)
        {
            gp_port_info_list_get_info( list, i, &info );
            plist.append( info.path );
        }
    }

    gp_port_info_list_free( list );
#else
    Q_UNUSED(plist);
#endif /* ENABLE_GPHOTO2 */
}

void GPCamera::getCameraSupportedPorts(const QString& model, QStringList& plist)
{
#ifdef ENABLE_GPHOTO2
    int i = 0;
    plist.clear();

    CameraAbilities      abilities;
    CameraAbilitiesList *abilList;
    GPContext           *context;

    context = gp_context_new();

    gp_abilities_list_new (&abilList);
    gp_abilities_list_load (abilList, context);
    i = gp_abilities_list_lookup_model (abilList, model.toLocal8Bit().data());
    gp_abilities_list_get_abilities (abilList, i, &abilities);
    gp_abilities_list_free (abilList);

    if (abilities.port & GP_PORT_SERIAL)
        plist.append("serial");

    if (abilities.port & GP_PORT_USB)
        plist.append("usb");

    gp_context_unref( context );
#else
    Q_UNUSED(model);
    Q_UNUSED(plist);
#endif /* ENABLE_GPHOTO2 */
}

int GPCamera::autoDetect(QString& model, QString& port)
{
#ifdef ENABLE_GPHOTO2
    CameraList          *camList;
    CameraAbilitiesList *abilList;
    GPPortInfoList      *infoList;
    const char          *camModel_, *camPort_;
    GPContext           *context;

    context = gp_context_new();
    gp_list_new(&camList);

    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, context);
    gp_port_info_list_new(&infoList);
    gp_port_info_list_load(infoList);
    gp_abilities_list_detect(abilList, infoList, camList, context);
    gp_abilities_list_free(abilList);
    gp_port_info_list_free(infoList);

    gp_context_unref(context);

    int count = gp_list_count(camList);

    if (count <= 0)
    {
        kDebug(50003) << "Failed to autodetect camera!" << endl;
        printGphotoErrorDescription(count);
        gp_list_free(camList);
        return -1;
    }

    camModel_ = 0;
    camPort_  = 0;

    for (int i = 0; i < count; ++i)
    {
        if (gp_list_get_name(camList, i, &camModel_) != GP_OK)
        {
            kDebug(50003) << "Failed to autodetect camera!" << endl;
            gp_list_free(camList);
            return -1;
        }

        if (gp_list_get_value(camList, i, &camPort_) != GP_OK)
        {
            kDebug(50003) << "Failed to autodetect camera!" << endl;
            gp_list_free(camList);
            return -1;
        }

        if (camModel_ && camPort_)
        {
            model = QString::fromLatin1(camModel_);
            port  = QString::fromLatin1(camPort_);
            gp_list_free(camList);
            return 0;
        }
    }

    kDebug(50003) << "Failed to autodetect camera!" << endl;
    gp_list_free(camList);
#else
    Q_UNUSED(model);
    Q_UNUSED(port);
#endif /* ENABLE_GPHOTO2 */
    return -1;
}

bool GPCamera::findConnectedUsbCamera(int vendorId, int productId, QString& model, QString& port)
{
#ifdef ENABLE_GPHOTO2
    CameraAbilitiesList *abilList;
    GPPortInfoList      *list;
    GPPortInfo           info;
    GPContext           *context;
    bool                 success;

    success = false;

    context = gp_context_new();

    // get list of all ports
    gp_port_info_list_new( &list );
    gp_port_info_list_load( list );

    int numPorts = gp_port_info_list_count( list );
    for (int i = 0 ; i < numPorts ; ++i)
    {
        // create a port object from info
        gp_port_info_list_get_info( list, i, &info );
        GPPort *gpport;
        gp_port_new(&gpport);
        gp_port_set_info(gpport, info);

        // check if device is connected to port
        if (gp_port_usb_find_device(gpport, vendorId, productId) == GP_OK)
        {
            CameraList          *camList;
            GPPortInfoList      *portinfo;

            // create three lists
            gp_list_new (&camList);
            gp_port_info_list_new( &portinfo );
            gp_abilities_list_new( &abilList );

            // append one port info to
            gp_port_info_list_append(portinfo, info);
            // get list of all supported cameras
            gp_abilities_list_load(abilList, context);
            // search for all supported cameras on one port
            gp_abilities_list_detect(abilList, portinfo, camList, context);
            int count = gp_list_count(camList);
            // get name and port of detected camera
            const char *model_str, *port_str;
            if (count > 0)
            {
                if (count > 1)
                {
                    kWarning(50003) << "More than one camera detected on port " << port
                                    << ". Due to restrictions in the GPhoto2 API, "
                                    << "only the first camera is used." << endl;
                }

                if (gp_list_get_name(camList, 0, &model_str) == GP_OK
                    && gp_list_get_value(camList, 0, &port_str) == GP_OK)
                {
                    model = QString::fromLatin1(model_str);
                    port  = QString::fromLatin1(port_str);

                    success = true;
                }
                else
                {
                    kError(50003) << "Failed to get information for the listed camera";
                }
            }

            gp_abilities_list_free( abilList );
            gp_port_info_list_free(portinfo);
            gp_list_free(camList);
        }
        gp_port_free(gpport);

        if (success)
            break;
    }

    gp_port_info_list_free( list );
    gp_context_unref( context );

    return success;
#else
    Q_UNUSED(vendorId);
    Q_UNUSED(productId);
    Q_UNUSED(model);
    Q_UNUSED(port);
    return false;
#endif /* ENABLE_GPHOTO2 */
}

}  // namespace Digikam
