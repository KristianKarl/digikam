/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to WikiMedia web service
 *
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_MEDIAWIKI_TALKER_H
#define DIGIKAM_MEDIAWIKI_TALKER_H

// Qt includes

#include <QString>
#include <QList>
#include <QMap>
#include <QUrl>

// KDE includes

#include <kjob.h>

// Local incudes

#include "dinfointerface.h"

namespace MediaWiki
{
    class Iface;
}

using namespace MediaWiki;

namespace Digikam
{

class MediaWikiTalker : public KJob
{
    Q_OBJECT

public:

    explicit MediaWikiTalker(DInfoInterface* const iface, Iface* const MediaWiki, QObject* const parent=0);
    ~MediaWikiTalker();

public:

    QString buildWikiText(const QMap<QString, QString>& info) const;

    void setImageMap(const QMap <QString,QMap <QString,QString> >& imageDesc);
    void start() Q_DECL_OVERRIDE;

Q_SIGNALS:

    void signalUploadProgress(int percent);
    void signalEndUpload();

public Q_SLOTS:

    void slotBegin();
    void slotUploadHandle(KJob* j = 0);
    void slotUploadProgress(KJob* job, unsigned long percent);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_MEDIAWIKI_TALKER_H
