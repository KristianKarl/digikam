/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_LIGHT_TABLE_WINDOW_H
#define DIGIKAM_LIGHT_TABLE_WINDOW_H

// Qt includes

#include <QString>
#include <QCloseEvent>
#include <QUrl>

// Local includes

#include "dxmlguiwindow.h"
#include "imageinfo.h"

namespace Digikam
{

class SlideShowSettings;

class LightTableWindow : public DXmlGuiWindow
{
    Q_OBJECT

public:

    ~LightTableWindow();

    static LightTableWindow* lightTableWindow();
    static bool              lightTableWindowCreated();

    void loadImageInfos(const ImageInfoList& list, const ImageInfo& imageInfoCurrent, bool addTo);
    void setLeftRightItems(const ImageInfoList& list, bool addTo);
    void refreshView();
    bool isEmpty() const;

    void toggleTag(int tagID);

Q_SIGNALS:

    void signalWindowHasMoved();

public Q_SLOTS:

    void slotFileChanged(const QString& filePath);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);

protected:

    void moveEvent(QMoveEvent* e);

private:

    void customizedFullScreenMode(bool set);
    void showSideBars(bool visible);
    void closeEvent(QCloseEvent* e);
    void showEvent(QShowEvent*);

    void deleteItem(bool permanently);
    void deleteItem(const ImageInfo& info, bool permanently);

    LightTableWindow();

private Q_SLOTS:

    void slotBackward();
    void slotForward();
    void slotFirst();
    void slotLast();

    void slotSetItemLeft();
    void slotSetItemRight();
    void slotSetItemOnLeftPanel(const ImageInfo&);
    void slotSetItemOnRightPanel(const ImageInfo&);
    void slotLeftDroppedItems(const ImageInfoList&);
    void slotRightDroppedItems(const ImageInfoList&);

    void slotLeftPanelLeftButtonClicked();
    void slotRightPanelLeftButtonClicked();

    void slotLeftPreviewLoaded(bool);
    void slotRightPreviewLoaded(bool);

    void slotLeftZoomFactorChanged(double);
    void slotRightZoomFactorChanged(double);

    void slotToggleOnSyncPreview(bool);
    void slotToggleSyncPreview();
    void slotToggleNavigateByPair();

    void slotDeleteItem();
    void slotDeleteItem(const ImageInfo&);

    void slotDeleteFinalItem();
    void slotDeleteFinalItem(const ImageInfo&);

    void slotRemoveItem();
    void slotRemoveItem(const ImageInfo&);

    void slotItemSelected(const ImageInfo&);
    void slotClearItemsList();

    void slotThumbbarDroppedItems(const QList<ImageInfo>&);

    void slotToggleColorManagedView();
    void slotComponentsInfo();
    void slotDBStat();

    void slotFileWithDefaultApplication();

    void slotRefreshStatusBar();

    void slotToggleLeftSideBar();
    void slotToggleRightSideBar();
    void slotPreviousLeftSideBarTab();
    void slotNextLeftSideBarTab();
    void slotPreviousRightSideBarTab();
    void slotNextRightSideBarTab();

    void slotRightSideBarActivateTitles();
    void slotRightSideBarActivateComments();
    void slotRightSideBarActivateAssignedTags();
    void slotLeftSideBarActivateTitles();
    void slotLeftSideBarActivateComments();
    void slotLeftSideBarActivateAssignedTags();

// -- Internal setup methods implemented in lighttablewindow_config.cpp ----------------------------------------

public:

    void applySettings();

private:

    void readSettings();
    void writeSettings();

public Q_SLOTS:

    void slotSetup();
    void slotColorManagementOptionsChanged();

    void slotThemeChanged();
    void slotApplicationSettingsChanged();

// -- Internal setup methods implemented in lighttablewindow_setup.cpp ----------------------------------------

private:

    void setupActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();

// -- Extra tool methods implemented in lighttablewindow_tools.cpp ----------------------------------------

private Q_SLOTS:

    void slotEditItem();
    void slotEditItem(const ImageInfo&);
    
    void slotEditMetadata();
    void slotEditGeolocation();
    void slotPrintCreator();
    void slotPresentation();

    void slotSlideShowAll();
    void slotLeftSlideShowManualFromCurrent();
    void slotRightSlideShowManualFromCurrent();
    void slotSlideShowLastItemUrl(const QUrl&);
    void slotSlideShowManualFrom(const ImageInfo&);
    void slotSlideShowBuilderComplete(const SlideShowSettings&);

// -- Export tools methods implemented in lighttablewindow_export.cpp -------------------------------------

private Q_SLOTS:

    void slotHtmlGallery();
    void slotCalendar();
    void slotExpoBlending();
    void slotPanorama();
    void slotVideoSlideshow();
    void slotSendByMail();
    void slotMediaServer();
    void slotExportTool();

// -- Import tools methods implemented in lighttablewindow_import.cpp -------------------------------------

private Q_SLOTS:

    void slotImportFromScanner();
    void slotImportedImagefromScanner(const QUrl& url);
    void slotImportTool();

// -- Internal private container --------------------------------------------------------------------

private:

    class Private;
    Private* const d;

    static LightTableWindow* m_instance;
};

} // namespace Digikam

#endif // DIGIKAM_LIGHT_TABLE_WINDOW_H
