////////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMAPP.CPP
//
//    Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////


#include <kdebug.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>

#include <qapplication.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>

extern "C" {
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
}

#include <jpegmetadata.h>

#include "albumdb.h"
#include "albummanager.h"
#include "scanlib.h"

/** @file scanlib.cpp*/

ScanLib::ScanLib()
{
    m_progressBar = new KProgressDialog(0);
    m_progressBar->setInitialSize(QSize(300,100), true);
    m_progressBar->hide();
    QWhatsThis::add( m_progressBar, i18n("This shows the progress of the "
        "scanning. During the scanning all files on disk are put in a "
        "database. This is needed for sorting on exif-date and speeds up "
        "overall performance of digiKam.") );
}

ScanLib::~ScanLib()
{
    delete m_progressBar;
}

void ScanLib::startScan()
{
    struct timeval tv1, tv2;

    gettimeofday(&tv1, 0);
    findFoldersWhichDoNotExist();
    gettimeofday(&tv2, 0);
    timing("Finding non-existing Albums",tv1, tv2);

    gettimeofday(&tv1, 0);
    findMissingItems();
    gettimeofday(&tv2, 0);
    timing("Finding items not in the database or disk",tv1, tv2);

    gettimeofday(&tv1, 0);
    updateItemsWithoutDate();
    gettimeofday(&tv2, 0);
    timing("Updating items without date",tv1, tv2);

    deleteStaleEntries();

    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->setSetting("Scanned",
                   QDateTime::currentDateTime().toString(Qt::ISODate));

}

void ScanLib::findFoldersWhichDoNotExist()
{
    QMap<QString, int> toBeDeleted;
    QString basePath(AlbumManager::instance()->getLibraryPath());

    AlbumDB* db = AlbumManager::instance()->albumDB();
    AlbumInfo::List aList = db->scanAlbums();
    for (AlbumInfo::List::iterator it = aList.begin(); it != aList.end(); ++it)
    {
        AlbumInfo info = *it;
        info.url = QDir::cleanDirPath(info.url);
        QFileInfo fi(basePath + info.url);
        if (!fi.exists() || !fi.isDir())
        {
            toBeDeleted[info.url] = info.id;
        }
    }

    kapp->processEvents();

    if (!toBeDeleted.isEmpty())
    {
        int rc = KMessageBox::warningYesNoList(   0,
            i18n("There seem to be an album in the database which does not "
                 "appear to be on disk. This album should be removed from "
                 "the database, but that means you might loose information, "
                 "because all images related to this album will be removed "
                 "from the database as well. digiKam can not continue "
                 "without removing the items from the database because all "
                 "views depends on the information in the database. Do you "
                 "want them to be removed from the database?",
                 "There seem to be %n albums in the database which do not "
                 "appear to be on disk. These albums should be removed from "
                 "the database, but that means you might loose information, "
                 "because all images related to this albums will be removed "
                 "from the database as well. digiKam can not continue "
                 "without removing the items from the database because all "
                 "views depends on the information in the database. Do you "
                 "want them to be removed from the database?",
                 toBeDeleted.count()),
            toBeDeleted.keys(),
            i18n("Albums are Missing"));

        if (rc != KMessageBox::Yes)
            exit(0);

        QMapIterator<QString,int> it;
        for (it = toBeDeleted.begin() ; it != toBeDeleted.end(); ++it)
        {
            kdDebug() << "Removing Album: " << it.key() << endl;
            db->deleteAlbum( it.data() );
        }
    }
}

void ScanLib::findMissingItems()
{
    QString albumPath = AlbumManager::instance()->getLibraryPath();
    albumPath = QDir::cleanDirPath(albumPath);

    m_progressBar->setAllowCancel( false );
    m_progressBar->showCancelButton (false );
    m_progressBar->progressBar()->setProgress( 0 );
    m_progressBar->setLabel(i18n("Scanning items..."));
    m_progressBar->progressBar()->
            setTotalSteps( countItemsInFolder( albumPath ) );
    m_progressBar->show();
    kapp->processEvents();

    QDir dir(albumPath);
    QStringList fileList(dir.entryList(QDir::Dirs));

    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->beginTransaction();    
    for (QStringList::iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        if ((*it) == "." || (*it) == "..")
            continue;

        allFiles( albumPath + '/' + (*it));
    }
    db->commitTransaction();    

    m_progressBar->hide();
    kapp->processEvents();
}

void ScanLib::updateItemsWithoutDate()
{
    AlbumDB* db = AlbumManager::instance()->albumDB();
    QStringList urls = db->getAllItemURLsWithoutDate();

    if (urls.isEmpty())
    {
        m_progressBar->progressBar()->setTotalSteps(1);
        m_progressBar->progressBar()->setProgress(1);
        m_progressBar->hide();
        return;
    }

    m_progressBar->setAllowCancel( false );
    m_progressBar->showCancelButton (false );
    m_progressBar->progressBar()->setProgress(0);
    m_progressBar->progressBar()->setTotalSteps(urls.count());
    m_progressBar->setLabel(i18n("Updating items..."));
    m_progressBar->show();
    kapp->processEvents();

    QString basePath = AlbumManager::instance()->getLibraryPath();
    basePath = QDir::cleanDirPath(basePath);

    db->beginTransaction();    

    int counter=0;
    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        m_progressBar->progressBar()->advance(1);
        ++counter;
        if ( counter % 30 == 0 )
        {
            kapp->processEvents();
        }

        QFileInfo fi(*it);
        QString albumURL = fi.dirPath();
        albumURL = QDir::cleanDirPath(albumURL.remove(basePath));
        
        int albumID = db->getOrCreateAlbumId(albumURL);

        if (albumID <= 0)
        {
            kdWarning() << "Album ID == -1: " << albumURL << endl;
        }
        
        if (fi.exists())
        {
            updateItemDate(albumURL, fi.fileName(), albumID);
        }
        else
        {
            QPair<QString, int> fileID = qMakePair(fi.fileName(),albumID);
            
            if (m_filesToBeDeleted.findIndex(fileID) == -1)
            {
                m_filesToBeDeleted.append(fileID);
            }
        }
    }

    db->commitTransaction();    
    
    m_progressBar->hide();
    kapp->processEvents();
}

int ScanLib::countItemsInFolder(const QString& directory)
{
    int items = 0;

    QDir dir( directory );
    if ( !dir.exists() or !dir.isReadable() )
        return 0;

    const QFileInfoList *list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    items += list->count();

    while ( (fi = it.current()) != 0 )
    {
        if ( fi->isDir() &&
             fi->fileName() != "." &&
             fi->fileName() != "..")
        {
            items += countItemsInFolder( fi->filePath() );
        }
        
        ++it;
    }

    return items;
}

void ScanLib::allFiles(const QString& directory)
{
    QDir dir( directory );
    if ( !dir.exists() or !dir.isReadable() )
    {
        kdWarning() << "Folder does not exist or is not readable: "
                    << directory << endl;
        return;
    }

    QString basePath = AlbumManager::instance()->getLibraryPath();
    basePath = QDir::cleanDirPath(basePath);
            
    QString albumURL = directory;
    albumURL = QDir::cleanDirPath(albumURL.remove(basePath));

    AlbumDB* db = AlbumManager::instance()->albumDB();

    int albumID = db->getOrCreateAlbumId(albumURL);

    if (albumID <= 0)
    {
        kdWarning() << "Album ID == -1: " << albumURL << endl;
    }
    
    QStringList filesInAlbum = db->getItemNamesInAlbum( albumID );

    QMap<QString, bool> filesFoundInDB;
    for (QStringList::iterator it = filesInAlbum.begin();
         it != filesInAlbum.end(); ++it)
    {
        filesFoundInDB.insert(*it, true);
    }

    const QFileInfoList *list = dir.entryInfoList();
    if (!list)
        return;

    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    m_progressBar->progressBar()->advance(list->count());
    kapp->processEvents();

    while ( (fi = it.current()) != 0 )
    {
        if ( fi->isFile())
        {
            if (filesFoundInDB.contains(fi->fileName()) )
            {
                filesFoundInDB.erase(fi->fileName());
            }
            else
            {
                storeItemInDatabase(albumURL, fi->fileName(), albumID);
            }
        }
        else if ( fi->isDir() && fi->fileName() != "." && fi->fileName() != "..")
        {
            allFiles( fi->filePath() );
        }
        
        ++it;
    }

    // Removing items from the db which we did not see on disk.
    if (!filesFoundInDB.isEmpty())
    {
        QMapIterator<QString,bool> it;
        for (it = filesFoundInDB.begin(); it != filesFoundInDB.end(); ++it)
        {
            if (m_filesToBeDeleted.findIndex(qMakePair(it.key(),albumID)) == -1)
            {
                m_filesToBeDeleted.append(qMakePair(it.key(),albumID));
            }
        }
    }
}

void ScanLib::storeItemInDatabase(const QString& albumURL,
                                  const QString& filename,
                                  int albumID)
{
    // Do not store items found in the root of the albumdb
    if (albumURL.isEmpty())
        return;

    QString comment;
    QDateTime datetime;
    
    QString filePath( AlbumManager::instance()->getLibraryPath());
    filePath += albumURL + '/' + filename;
    
    Digikam::readJPEGMetaData(filePath, comment, datetime);
    
    if ( !datetime.isValid() )
    {
        QFileInfo info( filePath );
        datetime = info.lastModified();
    }

    AlbumDB* dbstore = AlbumManager::instance()->albumDB();
    dbstore->addItem(albumID, filename, datetime,comment);
}

void ScanLib::updateItemDate(const QString& albumURL,
                             const QString& filename,
                             int albumID)
{
    QString comment;
    QDateTime datetime;
    
    QString filePath( AlbumManager::instance()->getLibraryPath());
    filePath += albumURL + '/' + filename;
    
    Digikam::readJPEGMetaData(filePath, comment, datetime);
    
    if ( !datetime.isValid() )
    {
        QFileInfo info( filePath );
        datetime = info.lastModified();
    }

    AlbumDB* dbstore = AlbumManager::instance()->albumDB();
    dbstore->setItemDate(albumID, filename, datetime);
}

void ScanLib::deleteStaleEntries()
{
    QStringList listToBeDeleted;
    QValueList< QPair<QString,int> >::iterator it;
    for (it = m_filesToBeDeleted.begin() ; it != m_filesToBeDeleted.end(); ++it)
    {
        listToBeDeleted.append((*it).first);
    }

    if ( !m_filesToBeDeleted.isEmpty() )
    {
        int rc = KMessageBox::warningYesNoList(   0,
          i18n("There seem to be an item in the database which does not "
               "appear to be on disk or is located in the root album of "
               "the path. This file should be removed from the "
               "database, but that means you might loose information. "
               "digiKam can not continue without removing the item from "
               "the database because all views depends on the information "
               "in the database. Do you want it to be removed from the "
               "database?",
               "There seem to be %n items in the database which do not "
               "appear to be on disk or are located in the root album of "
               "the path. These files should be removed from the "
               "database, but that means you might loose information. "
               "digiKam can not continue without removing these item from "
               "the database because all views depends on the information "
               "in the database. Do you want them to be removed from the "
               "database?",
               listToBeDeleted.count()),
          listToBeDeleted,
          i18n("Files are Missing"));

        if (rc != KMessageBox::Yes)
            exit(0);

        AlbumDB* db = AlbumManager::instance()->albumDB();
        db->beginTransaction();
        for (it = m_filesToBeDeleted.begin() ; it != m_filesToBeDeleted.end();
             ++it)
        {
            kdDebug() << "Removing: " << (*it).first << " in "
                      << (*it).second << endl;
            db->deleteItem( (*it).second, (*it).first );
        }
        db->commitTransaction();
    }
}

void ScanLib::timing(const QString& text, struct timeval tv1, struct timeval tv2)
{
    kdDebug() << "ScanLib: "
              << text + ": "
              << (((tv2.tv_sec-tv1.tv_sec)*1000000 +
                   (tv2.tv_usec-tv1.tv_usec))/1000)
              << " ms" << endl;
}
