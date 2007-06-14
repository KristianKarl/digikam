/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-07-09
 * Description : album item tool tip configuration setup tab
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

// // Local includes.

#include "albumsettings.h"
#include "setuptooltip.h"
#include "setuptooltip.moc"

namespace Digikam
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

        showAlbumNameBox  = 0;
        showCommentsBox   = 0;
        showTagsBox       = 0;
        showRatingBox     = 0;

        fileSettingBox    = 0;
        photoSettingBox   = 0;
        digikamSettingBox = 0;
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

    QCheckBox  *showAlbumNameBox;
    QCheckBox  *showCommentsBox;
    QCheckBox  *showTagsBox;
    QCheckBox  *showRatingBox;

    Q3VGroupBox *fileSettingBox;
    Q3VGroupBox *photoSettingBox;
    Q3VGroupBox *digikamSettingBox;
};

SetupToolTip::SetupToolTip(QWidget* parent)
            : QWidget(parent)
{
    d = new SetupToolTipPriv;
    Q3VBoxLayout *layout = new Q3VBoxLayout( parent, 0, KDialog::spacingHint() );

    d->showToolTipsBox = new QCheckBox(i18n("Show album items toolti&ps"), parent);
    Q3WhatsThis::add( d->showToolTipsBox, i18n("<p>Set this option to display image information when "
                                              "the mouse is hovered over an album item."));

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

    // --------------------------------------------------------

    d->digikamSettingBox = new Q3VGroupBox(i18n("digiKam Information"), parent);

    d->showAlbumNameBox = new QCheckBox(i18n("Show album name"), d->digikamSettingBox);
    Q3WhatsThis::add( d->showAlbumNameBox, i18n("<p>Set this option to display the album name."));

    d->showCommentsBox = new QCheckBox(i18n("Show picture comments"), d->digikamSettingBox);
    Q3WhatsThis::add( d->showCommentsBox, i18n("<p>Set this option to display picture comments."));

    d->showTagsBox = new QCheckBox(i18n("Show picture tags"), d->digikamSettingBox);
    Q3WhatsThis::add( d->showTagsBox, i18n("<p>Set this option to display picture tags."));

    d->showRatingBox = new QCheckBox(i18n("Show picture rating"), d->digikamSettingBox);
    Q3WhatsThis::add( d->showRatingBox, i18n("<p>Set this option to display the picture rating."));

    layout->addWidget(d->digikamSettingBox);
    layout->addStretch();

    // --------------------------------------------------------

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fileSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->photoSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->digikamSettingBox, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setShowToolTips(d->showToolTipsBox->isChecked());

    settings->setToolTipsShowFileName(d->showFileNameBox->isChecked());
    settings->setToolTipsShowFileDate(d->showFileDateBox->isChecked());
    settings->setToolTipsShowFileSize(d->showFileSizeBox->isChecked());
    settings->setToolTipsShowImageType(d->showImageTypeBox->isChecked());
    settings->setToolTipsShowImageDim(d->showImageDimBox->isChecked());

    settings->setToolTipsShowPhotoMake(d->showPhotoMakeBox->isChecked());
    settings->setToolTipsShowPhotoDate(d->showPhotoDateBox->isChecked());
    settings->setToolTipsShowPhotoFocal(d->showPhotoFocalBox->isChecked());
    settings->setToolTipsShowPhotoExpo(d->showPhotoExpoBox->isChecked());
    settings->setToolTipsShowPhotoMode(d->showPhotoModeBox->isChecked());
    settings->setToolTipsShowPhotoFlash(d->showPhotoFlashBox->isChecked());
    settings->setToolTipsShowPhotoWB(d->showPhotoWbBox->isChecked());

    settings->setToolTipsShowAlbumName(d->showAlbumNameBox->isChecked());
    settings->setToolTipsShowComments(d->showCommentsBox->isChecked());
    settings->setToolTipsShowTags(d->showTagsBox->isChecked());
    settings->setToolTipsShowRating(d->showRatingBox->isChecked());

    settings->saveSettings();
}

void SetupToolTip::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->showToolTipsBox->setChecked(settings->getShowToolTips());

    d->showFileNameBox->setChecked(settings->getToolTipsShowFileName());
    d->showFileDateBox->setChecked(settings->getToolTipsShowFileDate());
    d->showFileSizeBox->setChecked(settings->getToolTipsShowFileSize());
    d->showImageTypeBox->setChecked(settings->getToolTipsShowImageType());
    d->showImageDimBox->setChecked(settings->getToolTipsShowImageDim());

    d->showPhotoMakeBox->setChecked(settings->getToolTipsShowPhotoMake());
    d->showPhotoDateBox->setChecked(settings->getToolTipsShowPhotoDate());
    d->showPhotoFocalBox->setChecked(settings->getToolTipsShowPhotoFocal());
    d->showPhotoExpoBox->setChecked(settings->getToolTipsShowPhotoExpo());
    d->showPhotoModeBox->setChecked(settings->getToolTipsShowPhotoMode());
    d->showPhotoFlashBox->setChecked(settings->getToolTipsShowPhotoFlash());
    d->showPhotoWbBox->setChecked(settings->getToolTipsShowPhotoWB());

    d->showAlbumNameBox->setChecked(settings->getToolTipsShowAlbumName());
    d->showCommentsBox->setChecked(settings->getToolTipsShowComments());
    d->showTagsBox->setChecked(settings->getToolTipsShowTags());
    d->showRatingBox->setChecked(settings->getToolTipsShowRating());

    d->fileSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->photoSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->digikamSettingBox->setEnabled(d->showToolTipsBox->isChecked());
}

}  // namespace Digikam

