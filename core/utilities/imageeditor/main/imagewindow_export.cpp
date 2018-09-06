/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam image editor - Export tools
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

#include "imagewindow.h"
#include "imagewindow_p.h"

namespace Digikam
{

void ImageWindow::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    QPointer<HTMLWizard> w = new HTMLWizard(this, new DBInfoIface(this, d->thumbBar->allUrls()));

    w->exec();
    delete w;
#endif
}

void ImageWindow::slotCalendar()
{
    QPointer<CalWizard> w = new CalWizard(d->thumbBar->allUrls(), this);
    w->exec();
    delete w;
}

void ImageWindow::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(d->thumbBar->allUrls());
    PanoManager::instance()->run();
#endif
}

void ImageWindow::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    QPointer<VidSlideWizard> w = new VidSlideWizard(this, new DBInfoIface(this, d->thumbBar->allUrls()));
    w->exec();
    delete w;
#endif
}

void ImageWindow::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(d->thumbBar->allUrls());
    ExpoBlendingManager::instance()->run();
}

void ImageWindow::slotSendByMail()
{
    QPointer<MailWizard> w = new MailWizard(this, new DBInfoIface(this, d->thumbBar->allUrls()));
    w->exec();
    delete w;
}

void ImageWindow::slotMediaServer()
{
    DBInfoIface* const iface = new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools);
    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.
    iface->setObjectName(QLatin1String("SetupMediaServerIface"));

    QPointer<DMediaServerDlg> w = new DMediaServerDlg(this, iface);
    w->exec();
    delete w;
}

void ImageWindow::slotExportTool()
{
    QAction* const action = dynamic_cast<QAction*>(sender());
    int tool              = actionToWebService(action);

    if (tool != WSStarter::ExportUnknown)
    {
        WSStarter::exportToWebService(tool, new DBInfoIface(this, QList<QUrl>(),
                                                            ApplicationSettings::ImportExport), this);
    }
}

} // namespace Digikam
