/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : tool tip contents configuration setup tab
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// QT includes.

#include <qlayout.h>
#include <q3vgroupbox.h>
#include <qcheckbox.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <kconfig.h>

// // Local includes.

#include "albumsettings.h"
#include "setuptooltip.h"
#include "setuptooltip.moc"

namespace ShowFoto
{

class SetupToolTipPriv
{
public:

    SetupToolTipPriv()
    {
        showToolTipsBox   = 0;

        showFileNameBox   = 0;
        showFileDateBox   = 0;
        showFileSizeBox   = 0;
        showImageTypeBox  = 0;
        showImageDimBox   = 0;

        showPhotoMakeBox  = 0;
        showPhotoDateBox  = 0;
        showPhotoFocalBox = 0;
        showPhotoExpoBox  = 0;
        showPhotoModeBox  = 0;
        showPhotoFlashBox = 0;
        showPhotoWbBox    = 0;

        fileSettingBox    = 0;
        photoSettingBox   = 0;
    }

    QCheckBox  *showToolTipsBox;

    QCheckBox  *showFileNameBox;
    QCheckBox  *showFileDateBox;
    QCheckBox  *showFileSizeBox;
    QCheckBox  *showImageTypeBox;
    QCheckBox  *showImageDimBox;

    QCheckBox  *showPhotoMakeBox;
    QCheckBox  *showPhotoDateBox;
    QCheckBox  *showPhotoFocalBox;
    QCheckBox  *showPhotoExpoBox;
    QCheckBox  *showPhotoModeBox;
    QCheckBox  *showPhotoFlashBox;
    QCheckBox  *showPhotoWbBox;

    Q3VGroupBox *fileSettingBox;
    Q3VGroupBox *photoSettingBox;
};

SetupToolTip::SetupToolTip(QWidget* parent)
            : QWidget(parent)
{
    d = new SetupToolTipPriv;
    Q3VBoxLayout *layout = new Q3VBoxLayout( parent, 0, KDialog::spacingHint() );

    d->showToolTipsBox = new QCheckBox(i18n("Show Thumbbar items toolti&ps"), parent);
    Q3WhatsThis::add( d->showToolTipsBox, i18n("<p>Set this option to display image information when "
                                              "the mouse is hovered over a thumbbar item."));

    layout->addWidget(d->showToolTipsBox);

    // --------------------------------------------------------

    d->fileSettingBox = new Q3VGroupBox(i18n("File/Image Information"), parent);

    d->showFileNameBox = new QCheckBox(i18n("Show file name"), d->fileSettingBox);
    Q3WhatsThis::add( d->showFileNameBox, i18n("<p>Set this option to display image file name."));

    d->showFileDateBox = new QCheckBox(i18n("Show file date"), d->fileSettingBox);
    Q3WhatsThis::add( d->showFileDateBox, i18n("<p>Set this option to display image file date."));

    d->showFileSizeBox = new QCheckBox(i18n("Show file size"), d->fileSettingBox);
    Q3WhatsThis::add( d->showFileSizeBox, i18n("<p>Set this option to display image file size."));

    d->showImageTypeBox = new QCheckBox(i18n("Show image type"), d->fileSettingBox);
    Q3WhatsThis::add( d->showImageTypeBox, i18n("<p>Set this option to display image type."));

    d->showImageDimBox = new QCheckBox(i18n("Show image dimensions"), d->fileSettingBox);
    Q3WhatsThis::add( d->showImageDimBox, i18n("<p>Set this option to display image dimensions in pixels."));

    layout->addWidget(d->fileSettingBox);

    // --------------------------------------------------------

    d->photoSettingBox = new Q3VGroupBox(i18n("Photograph Information"), parent);

    d->showPhotoMakeBox = new QCheckBox(i18n("Show camera make and model"), d->photoSettingBox);
    Q3WhatsThis::add( d->showPhotoMakeBox, i18n("<p>Set this option to display the camera make and model "
                                               "with which the picture has been taken."));

    d->showPhotoDateBox = new QCheckBox(i18n("Show camera date"), d->photoSettingBox);
    Q3WhatsThis::add( d->showPhotoDateBox, i18n("<p>Set this option to display the date when the picture was taken."));

    d->showPhotoFocalBox = new QCheckBox(i18n("Show camera aperture and focal"), d->photoSettingBox);
    Q3WhatsThis::add( d->showPhotoFocalBox, i18n("<p>Set this option to display camera aperture and focal settings "
                     "used to take the picture."));

    d->showPhotoExpoBox = new QCheckBox(i18n("Show camera exposure and sensitivity"), d->photoSettingBox);
    Q3WhatsThis::add( d->showPhotoExpoBox, i18n("<p>Set this option to display camera exposure and sensitivity "
                     "used to take the picture."));

    d->showPhotoModeBox = new QCheckBox(i18n("Show camera mode and program"), d->photoSettingBox);
    Q3WhatsThis::add( d->showPhotoModeBox, i18n("<p>Set this option to display camera mode and program "
                     "used to take the picture."));

    d->showPhotoFlashBox = new QCheckBox(i18n("Show camera flash settings"), d->photoSettingBox);
    Q3WhatsThis::add( d->showPhotoFlashBox, i18n("<p>Set this option to display camera flash settings "
                     "used to take the picture."));

    d->showPhotoWbBox = new QCheckBox(i18n("Show camera white balance settings"), d->photoSettingBox);
    Q3WhatsThis::add( d->showPhotoWbBox, i18n("<p>Set this option to display camera white balance settings "
                     "used to take the picture."));

    layout->addWidget(d->photoSettingBox);
    layout->addStretch();

    // --------------------------------------------------------

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fileSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->photoSettingBox, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    d->showToolTipsBox->setChecked(config->readBoolEntry("Show ToolTips", true));

    d->showFileNameBox->setChecked(config->readBoolEntry("ToolTips Show File Name", true));
    d->showFileDateBox->setChecked(config->readBoolEntry("ToolTips Show File Date", false));
    d->showFileSizeBox->setChecked(config->readBoolEntry("ToolTips Show File Size", false));
    d->showImageTypeBox->setChecked(config->readBoolEntry("ToolTips Show Image Type", false));
    d->showImageDimBox->setChecked(config->readBoolEntry("ToolTips Show Image Dim", true));

    d->showPhotoMakeBox->setChecked(config->readBoolEntry("ToolTips Show Photo Make", true));
    d->showPhotoDateBox->setChecked(config->readBoolEntry("ToolTips Show Photo Date", true));
    d->showPhotoFocalBox->setChecked(config->readBoolEntry("ToolTips Show Photo Focal", true));
    d->showPhotoExpoBox->setChecked(config->readBoolEntry("ToolTips Show Photo Expo", true));
    d->showPhotoModeBox->setChecked(config->readBoolEntry("ToolTips Show Photo Mode", true));
    d->showPhotoFlashBox->setChecked(config->readBoolEntry("ToolTips Show Photo Flash", false));
    d->showPhotoWbBox->setChecked(config->readBoolEntry("ToolTips Show Photo WB", false));

    d->fileSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->photoSettingBox->setEnabled(d->showToolTipsBox->isChecked());
}

void SetupToolTip::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    config->writeEntry("Show ToolTips", d->showToolTipsBox->isChecked());

    config->writeEntry("ToolTips Show File Name", d->showFileNameBox->isChecked());
    config->writeEntry("ToolTips Show File Date", d->showFileDateBox->isChecked());
    config->writeEntry("ToolTips Show File Size", d->showFileSizeBox->isChecked());
    config->writeEntry("ToolTips Show Image Type", d->showImageTypeBox->isChecked());
    config->writeEntry("ToolTips Show Image Dim", d->showImageDimBox->isChecked());

    config->writeEntry("ToolTips Show Photo Make", d->showPhotoMakeBox->isChecked());
    config->writeEntry("ToolTips Show Photo Date", d->showPhotoDateBox->isChecked());
    config->writeEntry("ToolTips Show Photo Focal", d->showPhotoFocalBox->isChecked());
    config->writeEntry("ToolTips Show Photo Expo", d->showPhotoExpoBox->isChecked());
    config->writeEntry("ToolTips Show Photo Mode", d->showPhotoModeBox->isChecked());
    config->writeEntry("ToolTips Show Photo Flash", d->showPhotoFlashBox->isChecked());
    config->writeEntry("ToolTips Show Photo WB", d->showPhotoWbBox->isChecked());

    config->sync();
}

}  // namespace ShowFoto

