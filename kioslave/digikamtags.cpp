/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a kio-slave to process tag query on 
 *               digiKam albums.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// C++ includes.

#include <cstdlib>

// KDE includes.

#include <kinstance.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/global.h>

// Local includes.

#include "digikam_export.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "imagelister.h"
#include "imagelisterreceiver.h"
#include "digikamtags.h"
//Added by qt3to4:
#include <Q3CString>

kio_digikamtagsProtocol::kio_digikamtagsProtocol(const Q3CString &pool_socket,
                                                 const Q3CString &app_socket)
                       : SlaveBase("kio_digikamtags", pool_socket, app_socket)
{
}

kio_digikamtagsProtocol::~kio_digikamtagsProtocol()
{
}

void kio_digikamtagsProtocol::special(const QByteArray& data)
{
    KURL    kurl;
    QString filter;
    int     getDimensions;

    QDataStream ds(data, QIODevice::ReadOnly);
    ds >> kurl;
    ds >> filter;
    ds >> getDimensions;

    Digikam::DatabaseUrl dbUrl(kurl);
    Digikam::DatabaseAccess::setParameters(dbUrl);

    Digikam::ImageLister lister;
    // send data every 200 images to be more responsive
    Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
    lister.list(&receiver, kurl, filter, getDimensions);
    // send rest
    receiver.sendData();

    finished();
}

/* KIO slave registration */

extern "C"
{
    DIGIKAM_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalogue("digikam");
        KInstance instance( "kio_digikamtags" );
        ( void ) KGlobal::locale();

        kdDebug() << "*** kio_digikamtag started ***" << endl;

        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamtags  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        kio_digikamtagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kdDebug() << "*** kio_digikamtags finished ***" << endl;
        return 0;
    }
}

