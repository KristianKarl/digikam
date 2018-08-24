/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - Export tools
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfoto.h"
#include "showfoto_p.h"

namespace ShowFoto
{

void ShowFoto::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    QPointer<HTMLWizard> w = new HTMLWizard(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
#endif
}

void ShowFoto::slotCalendar()
{
    QPointer<CalWizard> w = new CalWizard(d->thumbBar->urls(), this);
    w->exec();
    delete w;
}

void ShowFoto::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(d->thumbBar->urls());
    PanoManager::instance()->run();
#endif
}

void ShowFoto::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(d->thumbBar->urls());
    ExpoBlendingManager::instance()->run();
}

void ShowFoto::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    QPointer<VidSlideWizard> w = new VidSlideWizard(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
#endif
}

void ShowFoto::slotSendByMail()
{
    QPointer<MailWizard> w = new MailWizard(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
}

void ShowFoto::slotMediaServer()
{
    QPointer<DMediaServerDlg> w = new DMediaServerDlg(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
}

void ShowFoto::slotExportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_exportDropboxAction)
    {
        QPointer<DBWindow> w = new DBWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    if (tool == m_exportOnedriveAction)
    {
        QPointer<ODWindow> w = new ODWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    if (tool == m_exportPinterestAction)
    {
        QPointer<PWindow> w = new PWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    if (tool == m_exportBoxAction)
    {
        QPointer<BOXWindow> w = new BOXWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportFacebookAction)
    {
        QPointer<FbWindow> w = new FbWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportFlickrAction)
    {
        QPointer<FlickrWindow> w = new FlickrWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportGdriveAction)
    {
        QPointer<GSWindow> w = new GSWindow(new DMetaInfoIface(this, d->thumbBar->urls()),
                   this, QLatin1String("googledriveexport"));
        w->exec();
        delete w;
    }
    else if (tool == m_exportGphotoAction)
    {
        QPointer<GSWindow> w = new GSWindow(new DMetaInfoIface(this, d->thumbBar->urls()),
                   this, QLatin1String("googlephotoexport"));
        w->exec();
        delete w;
    }
    else if (tool == m_exportImageshackAction)
    {
        QPointer<ImageShackWindow> w = new ImageShackWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportImgurAction)
    {
        QPointer<ImgurWindow> w = new ImgurWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportPiwigoAction)
    {
        QPointer<PiwigoWindow> w = new PiwigoWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportRajceAction)
    {
        QPointer<RajceWindow> w = new RajceWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportSmugmugAction)
    {
        QPointer<SmugWindow> w = new SmugWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportYandexfotkiAction)
    {
        QPointer<YFWindow> w = new YFWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }

#ifdef HAVE_MEDIAWIKI
    else if (tool == m_exportMediawikiAction)
    {
        QPointer<MediaWikiWindow> w = new MediaWikiWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
#endif

#ifdef HAVE_VKONTAKTE
    else if (tool == m_exportVkontakteAction)
    {
        QPointer<VKWindow> w = new VKWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
#endif

#ifdef HAVE_KIO
    else if (tool == m_exportFileTransferAction)
    {
        QPointer<FTExportWindow> w = new FTExportWindow(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w->exec();
        delete w;
    }
#endif
}

} // namespace ShowFoto
